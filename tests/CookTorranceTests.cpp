#include "SceneXmlLoader.h"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace
{
    void Check(bool condition, const char* message)
    {
        if (!condition) throw std::runtime_error(message);
    }

    Color ReferenceBRDF(Color albedo, float roughness, float metallic, Vector3f wo, Vector3f wi)
    {
        const glm::dvec3 v(wo), l(wi);
        const glm::dvec3 h = glm::normalize(v + l);
        const double r = std::max(0.03, double(roughness));
        const double alpha2 = r * r * r * r;
        const double pi = 3.14159265358979323846;
        const double denominator = h.z * h.z * (alpha2 - 1) + 1;
        const double D = alpha2 / std::max(pi * denominator * denominator, 1e-8);
        const double k = (r + 1) * (r + 1) / 8;
        const double G = v.z / (v.z * (1 - k) + k) * l.z / (l.z * (1 - k) + k);
        const auto f0 = glm::mix(glm::dvec3(.04), glm::dvec3(albedo), double(metallic));
        const auto F = f0 + (glm::dvec3(1) - f0) * std::pow(1 - glm::dot(v, h), 5);
        return Color((1 - double(metallic)) * (glm::dvec3(1) - F) * glm::dvec3(albedo) / pi +
            D * G * F / (4 * v.z * l.z));
    }

    void Near(Color actual, Color expected, const char* message)
    {
        Check(std::isfinite(actual.x) && std::isfinite(actual.y) && std::isfinite(actual.z), message);
        Check(glm::length(actual - expected) < .003f * (1 + glm::length(expected)), message);
    }
}

int main(int argc, char** argv)
{
    try
    {
        Check(argc == 3, "Expected source and fixture directories");
        const std::filesystem::path root(argv[1]), scratch(argv[2]);
        std::filesystem::create_directories(scratch);
        const Color base(.7f, .3f, .1f);
        const Vector3f n(0, 0, 1), wi = glm::normalize(Vector3f(.3f, .2f, 1));
        for (float roughness : {0.f, .03f, .3f, 1.f})
        {
            for (float metallic : {0.f, .5f, 1.f})
            {
                CookTorranceMaterial material(base, nullptr, roughness, nullptr, metallic, nullptr);
                Check(!material.IsSpecular(), "CookTorrance must receive direct lighting");
                Near(material.BRDF(Vector2f(0), n, wi), ReferenceBRDF(base, roughness, metallic, n, wi),
                    "Lecture D/F/G formula mismatch");
                Near(material.BRDF(Vector2f(0), wi, n), material.BRDF(Vector2f(0), n, wi), "Reciprocity failed");
                Near(material.BRDF(Vector2f(0), n, -n), Color(0), "Below-surface BRDF must be zero");
            }
        }
        CookTorranceMaterial material(base, nullptr, .6f, nullptr, .4f, nullptr);
        for (int i = 0; i < 20000; ++i)
        {
            float pdf = -1;
            const auto sample = material.SampleWi(Vector2f(0), n, pdf);
            Check(std::isfinite(pdf) && pdf >= 0, "Invalid sampler PDF");
            if (pdf == 0) continue;
            Check(sample.z > 0 && std::abs(glm::length(sample) - 1) < 1e-5f, "Invalid sampled direction");
            const auto h = glm::normalize(n + sample);
            const float a2 = std::pow(.6f, 4.f);
            const float den = h.z * h.z * (a2 - 1.f) + 1.f;
            const float D = a2 / std::max(PI * den * den, 1e-8f);
            const float expected = .5f * (sample.z * INV_PI + D * h.z / (4 * glm::dot(n, h)));
            Check(std::abs(pdf - expected) < .0001f * (1 + expected), "PDF must use the lecture's 50/50 mixture");
        }
        auto scene = LoadSceneFromXml((root / "scenes/scene18.xml").string(), 64, 36);
        for (const char* name : {"M_CookTorrance_Tile", "M_CookTorrance_Ball1", "M_CookTorrance_Ball2"})
        {
            auto* parsed = dynamic_cast<CookTorranceMaterial*>(scene->GetMaterial(name));
            Check(parsed && parsed->GetNormalMap(), "Lecture scene material or normal map missing");
        }
        Check(scene->GetLights().size() == 2, "Lecture scene must have an area light and the front point light");
        Ray centerRay;
        centerRay.origin = Vector3f(0, .64f, -2.58f);
        centerRay.direction = Vector3f(0, 0, 1);
        Intersection centerHit{};
        Check(scene->Intersect(centerRay, centerHit) &&
            glm::distance(centerHit.position, Vector3f(0, .64f, 1)) < 1e-4f,
            "Lecture back wall must be in world space at z=1");
        auto* area = dynamic_cast<AreaLight*>(scene->GetLights().front());
        Check(area != nullptr, "Lecture emitter was not registered");
        Vector3f lightPosition;
        float lightPdf = 0;
        area->GetRadiance(Vector3f(0, 0, 0), lightPosition, lightPdf);
        Check(std::abs(lightPosition.y - 1.499f) < 1e-4f && lightPdf > 0,
            "Transformed ceiling area-light sample is misplaced");

        const std::string start = "<Scene><Camera><Position>0,0,-3</Position><Target>0,0,0</Target><Up>0,1,0</Up></Camera><Materials><Material><Name>m</Name><Type>CookTorrance</Type><Albedo>0.7,0.3,0.1</Albedo>";
        const std::string end = "</Material></Materials><SceneObjects><SceneObject><Transform/><Material>m</Material><Primitives><Sphere><Radius>1</Radius></Sphere></Primitives></SceneObject></SceneObjects></Scene>";
        const auto file = scratch / "cook.xml";
        const auto scalar = scratch / "scalar.ppm";
        { std::ofstream out(scalar, std::ios::binary); out << "P6\n1 1\n255\n" << std::string(3, char(128)); }
        { std::ofstream out(file); out << start << "<Roughness>0.8</Roughness><RoughnessMap>scalar.ppm</RoughnessMap><Metallic>0.6</Metallic><MetallicMap>scalar.ppm</MetallicMap>" << end; }
        auto mapped = LoadSceneFromXml(file.string(), 64, 36);
        Near(mapped->GetMaterial("m")->BRDF(Vector2f(.5f), n, wi),
            ReferenceBRDF(base, .8f * 128 / 255, .6f * 128 / 255, n, wi), "Scalar maps not multiplied into parameters");
        for (const char* bad : {"<Roughness>-1</Roughness>", "<Metallic>2</Metallic>", "<MetallicMap>missing.png</MetallicMap>"})
        {
            { std::ofstream out(file); out << start << bad << end; }
            bool rejected = false;
            try { LoadSceneFromXml(file.string(), 64, 36); }
            catch (const std::runtime_error&) { rejected = true; }
            Check(rejected, "Invalid XML accepted");
        }
        for (const auto& entry : std::filesystem::directory_iterator(root / "scenes"))
            if (entry.path().extension() == ".xml") LoadSceneFromXml(entry.path().string(), 64, 36);
        std::cout << "PASS: lecture D/F/G, 50/50 sampler PDF, texture maps, XML validation and existing scenes.\n";
        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
