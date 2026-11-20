#include "SceneXmlLoader.h"

#include <cmath>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>

#include "Disk.h"
#include "Sphere.h"
#include "Triangle.h"
#include "../third_party/tinyxml2/tinyxml2.h"

namespace
{
    using tinyxml2::XMLElement;

    std::runtime_error SceneError(const XMLElement* element, const std::string& message)
    {
        std::ostringstream output;
        output << "XML line " << element->GetLineNum() << " <" << element->Name() << ">: " << message;
        return std::runtime_error(output.str());
    }

    void CheckAttributes(const XMLElement* element, std::initializer_list<const char*> allowed)
    {
        for (const tinyxml2::XMLAttribute* attribute = element->FirstAttribute(); attribute; attribute = attribute->Next())
        {
            bool known = false;
            for (const char* name : allowed)
            {
                if (std::string(attribute->Name()) == name)
                {
                    known = true;
                    break;
                }
            }
            if (!known)
            {
                throw SceneError(element, "unknown attribute '" + std::string(attribute->Name()) + "'");
            }
        }
    }

    Vector3f ReadVector3(const XMLElement* element, const char* name)
    {
        const char* text = element->Attribute(name);
        if (!text)
        {
            throw SceneError(element, "missing attribute '" + std::string(name) + "'");
        }

        std::istringstream input(text);
        Vector3f value;
        if (!(input >> value.x >> value.y >> value.z) || !(input >> std::ws).eof() ||
            !std::isfinite(value.x) || !std::isfinite(value.y) || !std::isfinite(value.z))
        {
            throw SceneError(element, "attribute '" + std::string(name) + "' must contain three finite numbers");
        }
        return value;
    }

    float ReadFloat(const XMLElement* element, const char* name)
    {
        const char* text = element->Attribute(name);
        if (!text)
        {
            throw SceneError(element, "missing attribute '" + std::string(name) + "'");
        }

        std::istringstream input(text);
        float value = 0.0f;
        if (!(input >> value) || !(input >> std::ws).eof() || !std::isfinite(value))
        {
            throw SceneError(element, "attribute '" + std::string(name) + "' must be a finite number");
        }
        return value;
    }

    float ReadPositiveFloat(const XMLElement* element, const char* name)
    {
        const float value = ReadFloat(element, name);
        if (value <= 0.0f)
        {
            throw SceneError(element, "attribute '" + std::string(name) + "' must be positive");
        }
        return value;
    }

    Vector3f ReadDirection(const XMLElement* element)
    {
        const Vector3f direction = ReadVector3(element, "direction");
        if (glm::length(direction) <= 1e-6f)
        {
            throw SceneError(element, "attribute 'direction' must be nonzero");
        }
        return direction;
    }

    Color ReadColor(const XMLElement* element, const char* name)
    {
        const Color color = ReadVector3(element, name);
        if (color.r < 0.0f || color.g < 0.0f || color.b < 0.0f)
        {
            throw SceneError(element, "attribute '" + std::string(name) + "' must be nonnegative");
        }
        return color;
    }

    Vector3f ReadAttenuations(const XMLElement* element)
    {
        const Vector3f attenuations = ReadVector3(element, "attenuations");
        if (attenuations.x < 0.0f || attenuations.y < 0.0f || attenuations.z < 0.0f ||
            (attenuations.x == 0.0f && attenuations.y == 0.0f && attenuations.z == 0.0f))
        {
            throw SceneError(element, "'attenuations' needs nonnegative quadratic/linear/constant terms, with at least one positive");
        }
        return attenuations;
    }

    void CheckNoChildren(const XMLElement* element)
    {
        if (element->FirstChildElement())
        {
            throw SceneError(element, "unexpected child element");
        }
    }

    Camera ParseCamera(const XMLElement* element, int width, int height)
    {
        CheckAttributes(element, { "position", "target", "up", "fov", "near", "far" });
        CheckNoChildren(element);

        const Vector3f position = ReadVector3(element, "position");
        const Vector3f target = ReadVector3(element, "target");
        const Vector3f up = ReadVector3(element, "up");
        const float fov = ReadFloat(element, "fov");
        const float nearPlane = ReadPositiveFloat(element, "near");
        const float farPlane = ReadFloat(element, "far");

        if (fov <= 0.0f || fov >= 180.0f || farPlane <= nearPlane ||
            glm::length(glm::cross(target - position, up)) <= 1e-6f)
        {
            throw SceneError(element, "invalid camera direction, FOV, or clipping planes");
        }

        Camera camera;
        camera.Initialize(position, target, up, glm::radians(fov), nearPlane, farPlane, width, height);
        return camera;
    }

    void CreateLambertMaterial(const XMLElement* element, Scene& scene, const std::string& name, const Color& albedo)
    {
        if (scene.GetMaterial(name))
        {
            throw SceneError(element, "duplicate material name '" + name + "'");
        }
        if (glm::any(glm::lessThan(albedo, Color(0.0f))) || glm::any(glm::greaterThan(albedo, Color(1.0f))))
        {
            throw SceneError(element, "albedo components must be between 0 and 1");
        }
        scene.CreateMaterial<LambertMaterial>(name, albedo);
    }

    void ParseMaterial(const XMLElement* element, Scene& scene)
    {
        CheckAttributes(element, { "name", "type", "albedo" });
        CheckNoChildren(element);

        const char* name = element->Attribute("name");
        const char* type = element->Attribute("type");
        if (!name || std::string(name).find_first_not_of(" \t\r\n") == std::string::npos)
        {
            throw SceneError(element, "material needs a nonempty 'name'");
        }
        if (!type)
        {
            throw SceneError(element, "material needs a 'type'");
        }
        if (std::string(type) != "lambert")
        {
            throw SceneError(element, "unsupported material type '" + std::string(type) + "'");
        }

        CreateLambertMaterial(element, scene, name, ReadVector3(element, "albedo"));
    }

    void ParseObject(const XMLElement* element, Scene& scene)
    {
        CheckAttributes(element, { "position", "rotation", "scale", "material" });
        const char* materialName = element->Attribute("material");
        if (!materialName)
        {
            throw SceneError(element, "missing attribute 'material'");
        }
        Material* material = scene.GetMaterial(materialName);
        if (!material)
        {
            throw SceneError(element, "unknown material '" + std::string(materialName) + "'");
        }
        const Vector3f position = ReadVector3(element, "position");
        const Vector3f rotationDegrees = ReadVector3(element, "rotation");
        const float scale = ReadPositiveFloat(element, "scale");
        const Vector3f rotation = glm::radians(rotationDegrees);

        if (!element->FirstChildElement())
        {
            throw SceneError(element, "an object needs at least one primitive");
        }

        SceneObject* object = scene.CreateSceneObject(position, rotation, scale);
        object->SetMaterial(material);
        for (const XMLElement* primitive = element->FirstChildElement(); primitive; primitive = primitive->NextSiblingElement())
        {
            const std::string type = primitive->Name();
            CheckNoChildren(primitive);
            if (type == "triangle")
            {
                CheckAttributes(primitive, { "v0", "v1", "v2" });
                const Vector3f v0 = ReadVector3(primitive, "v0");
                const Vector3f v1 = ReadVector3(primitive, "v1");
                const Vector3f v2 = ReadVector3(primitive, "v2");
                if (glm::length(glm::cross(v1 - v0, v2 - v0)) <= 1e-6f)
                {
                    throw SceneError(primitive, "triangle vertices must not be collinear");
                }
                object->CreatePrimitive<Triangle>(v0, v1, v2);
            }
            else if (type == "sphere")
            {
                CheckAttributes(primitive, { "radius" });
                object->CreatePrimitive<Sphere>(ReadPositiveFloat(primitive, "radius"));
            }
            else if (type == "disk")
            {
                CheckAttributes(primitive, { "radius" });
                object->CreatePrimitive<Disk>(ReadPositiveFloat(primitive, "radius"));
            }
            else
            {
                throw SceneError(primitive, "unsupported primitive type");
            }
        }
    }

    void ParseLight(const XMLElement* element, Scene& scene)
    {
        CheckNoChildren(element);
        const char* type = element->Attribute("type");
        if (!type)
        {
            throw SceneError(element, "missing attribute 'type'");
        }

        if (std::string(type) == "directional")
        {
            CheckAttributes(element, { "type", "direction", "radiance" });
            const Vector3f direction = ReadDirection(element);
            const Color radiance = ReadColor(element, "radiance");
            scene.CreateLight<DirectionalLight>(direction, radiance);
        }
        else if (std::string(type) == "point")
        {
            CheckAttributes(element, { "type", "position", "intensity", "attenuations" });
            const Vector3f position = ReadVector3(element, "position");
            const Color intensity = ReadColor(element, "intensity");
            const Vector3f attenuations = ReadAttenuations(element);
            scene.CreateLight<PointLight>(position, intensity, attenuations);
        }
        else if (std::string(type) == "spot")
        {
            CheckAttributes(element, { "type", "position", "direction", "intensity", "innerAngle", "outerAngle", "attenuations" });
            const Vector3f position = ReadVector3(element, "position");
            const Vector3f direction = ReadDirection(element);
            const Color intensity = ReadColor(element, "intensity");
            const float innerAngle = ReadFloat(element, "innerAngle");
            const float outerAngle = ReadFloat(element, "outerAngle");
            const Vector3f attenuations = ReadAttenuations(element);
            if (innerAngle < 0.0f || innerAngle >= outerAngle || outerAngle >= 180.0f)
            {
                throw SceneError(element, "spot angles need 0 <= innerAngle < outerAngle < 180 degrees");
            }
            scene.CreateLight<SpotLight>(position, direction, intensity,
                glm::radians(innerAngle), glm::radians(outerAngle), attenuations);
        }
        else
        {
            throw SceneError(element, "unsupported light type '" + std::string(type) + "'");
        }
    }

    void CheckChildren(const XMLElement* element, std::initializer_list<const char*> allowed, bool unique = true)
    {
        CheckAttributes(element, {});
        std::set<std::string> seen;
        for (const XMLElement* child = element->FirstChildElement(); child; child = child->NextSiblingElement())
        {
            const std::string name = child->Name();
            bool known = false;
            for (const char* field : allowed)
            {
                if (name == field)
                {
                    known = true;
                    break;
                }
            }
            if (!known || (unique && !seen.insert(name).second))
            {
                throw SceneError(child, "unknown or duplicate element");
            }
            CheckAttributes(child, {});
        }
    }

    void CheckValueFields(const XMLElement* element, std::initializer_list<const char*> allowed)
    {
        CheckChildren(element, allowed);
        for (const XMLElement* child = element->FirstChildElement(); child; child = child->NextSiblingElement())
            CheckNoChildren(child);
    }

    const XMLElement* RequireChild(const XMLElement* element, const char* name)
    {
        const XMLElement* child = element->FirstChildElement(name);
        if (!child) throw SceneError(element, "missing <" + std::string(name) + ">");
        return child;
    }

    const XMLElement* RequireChildValue(const XMLElement* element, const char* name)
    {
        const XMLElement* child = RequireChild(element, name);
        CheckNoChildren(child);
        if (!child->GetText())
        {
            throw SceneError(element, "missing <" + std::string(name) + "> value");
        }
        return child;
    }

    std::string ReadChildText(const XMLElement* element, const char* name)
    {
        const XMLElement* child = RequireChildValue(element, name);
        const std::string value = child->GetText();
        const std::size_t first = value.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
        {
            throw SceneError(child, "expected nonempty text");
        }
        const std::size_t last = value.find_last_not_of(" \t\r\n");
        return value.substr(first, last - first + 1);
    }

    Vector3f ReadElementVector3(const XMLElement* child)
    {
        CheckAttributes(child, {});
        CheckNoChildren(child);
        if (!child->GetText()) throw SceneError(child, "expected three finite numbers");
        std::string text = child->GetText();
        for (char& character : text)
        {
            if (character == ',') character = ' ';
        }
        std::istringstream input(text);
        Vector3f value;
        if (!(input >> value.x >> value.y >> value.z) || !(input >> std::ws).eof() ||
            !std::isfinite(value.x) || !std::isfinite(value.y) || !std::isfinite(value.z))
        {
            throw SceneError(child, "expected three finite numbers");
        }
        return value;
    }

    Vector3f ReadChildVector3(const XMLElement* element, const char* name)
    {
        return ReadElementVector3(RequireChildValue(element, name));
    }

    float ReadChildFloat(const XMLElement* element, const char* name)
    {
        const XMLElement* child = RequireChildValue(element, name);
        std::istringstream input(child->GetText());
        float value = 0.0f;
        if (!(input >> value) || !(input >> std::ws).eof() || !std::isfinite(value))
        {
            throw SceneError(child, "expected a finite number");
        }
        return value;
    }

    float ReadOptionalChildFloat(const XMLElement* element, const char* name, float fallback)
    {
        return element->FirstChildElement(name) ? ReadChildFloat(element, name) : fallback;
    }

    void ParseNestedMaterials(const XMLElement* element, Scene& scene)
    {
        CheckChildren(element, { "Material" }, false);
        for (const XMLElement* material = element->FirstChildElement(); material; material = material->NextSiblingElement())
        {
            const std::string name = ReadChildText(material, "Name");
            const std::string type = ReadChildText(material, "Type");
            if (type == "Lambert")
            {
                CheckValueFields(material, { "Name", "Type", "Albedo" });
                CreateLambertMaterial(material, scene, name, ReadChildVector3(material, "Albedo"));
            }
            else if (type == "ConductorSpecular")
            {
                CheckValueFields(material, { "Name", "Type", "Eta", "AbsorptionCoef", "ReflectionColor" });
                if (scene.GetMaterial(name))
                {
                    throw SceneError(material, "duplicate material name '" + name + "'");
                }

                const Color eta = ReadChildVector3(material, "Eta");
                const Color absorption = ReadChildVector3(material, "AbsorptionCoef");
                const Color reflection = ReadChildVector3(material, "ReflectionColor");
                if (glm::any(glm::lessThanEqual(eta, Color(0.0f))))
                {
                    throw SceneError(material, "<Eta> components must be positive");
                }
                if (glm::any(glm::lessThan(absorption, Color(0.0f))))
                {
                    throw SceneError(material, "<AbsorptionCoef> components must be nonnegative");
                }
                if (glm::any(glm::lessThan(reflection, Color(0.0f))) ||
                    glm::any(glm::greaterThan(reflection, Color(1.0f))))
                {
                    throw SceneError(material, "<ReflectionColor> components must be between 0 and 1");
                }

                scene.CreateMaterial<ConductorSpecularMaterial>(name, eta, absorption, reflection);
            }
            else if (type == "DielectricSpecular")
            {
                CheckValueFields(material, { "Name", "Type", "Eta", "TransmissionColor" });
                if (scene.GetMaterial(name))
                {
                    throw SceneError(material, "duplicate material name '" + name + "'");
                }

                const float eta = ReadOptionalChildFloat(material, "Eta", 1.5f);
                const Color transmission = ReadChildVector3(material, "TransmissionColor");
                if (eta <= 0.0f)
                {
                    throw SceneError(material, "<Eta> must be positive");
                }
                if (glm::any(glm::lessThan(transmission, Color(0.0f))) ||
                    glm::any(glm::greaterThan(transmission, Color(1.0f))))
                {
                    throw SceneError(material, "<TransmissionColor> components must be between 0 and 1");
                }

                scene.CreateMaterial<DielectricSpecularMaterial>(name, eta, transmission);
            }
            else
            {
                throw SceneError(material, "unsupported material type '" + type + "'");
            }
        }
    }

    Color ReadLightColor(const XMLElement* element, const char* name)
    {
        const Color value = ReadChildVector3(element, name);
        if (value.r < 0.0f || value.g < 0.0f || value.b < 0.0f)
        {
            throw SceneError(element, "<" + std::string(name) + "> must be nonnegative");
        }
        return value;
    }

    Vector3f ReadLightDirection(const XMLElement* element)
    {
        const Vector3f direction = ReadChildVector3(element, "Direction");
        if (glm::length(direction) <= 1e-6f)
        {
            throw SceneError(element, "<Direction> must be nonzero");
        }
        return direction;
    }

    Vector3f ReadLightAttenuations(const XMLElement* element)
    {
        // Quadratic, linear, constant order, matching the light classes.
        const Vector3f coefficients = ReadChildVector3(element, "Attenuations");
        if (coefficients.x < 0.0f || coefficients.y < 0.0f || coefficients.z < 0.0f ||
            (coefficients.x == 0.0f && coefficients.y == 0.0f && coefficients.z == 0.0f))
        {
            throw SceneError(element, "<Attenuations> needs nonnegative quadratic/linear/constant terms, with at least one positive");
        }
        return coefficients;
    }

    void ParseNestedLight(const XMLElement* element, Scene& scene)
    {
        const std::string type = element->Name();
        if (type == "DirectionalLight")
        {
            CheckValueFields(element, { "Direction", "Radiance" });
            const Vector3f direction = ReadLightDirection(element);
            const Color radiance = ReadLightColor(element, "Radiance");
            scene.CreateLight<DirectionalLight>(direction, radiance);
        }
        else if (type == "PointLight")
        {
            CheckValueFields(element, { "Position", "Intensity", "Attenuations" });
            const Vector3f position = ReadChildVector3(element, "Position");
            const Color intensity = ReadLightColor(element, "Intensity");
            const Vector3f attenuations = ReadLightAttenuations(element);
            scene.CreateLight<PointLight>(position, intensity, attenuations);
        }
        else if (type == "SpotLight")
        {
            CheckValueFields(element, { "Position", "Direction", "Intensity",
                "InnerAngle", "OuterAngle", "Attenuations" });
            const Vector3f position = ReadChildVector3(element, "Position");
            const Vector3f direction = ReadLightDirection(element);
            const Color intensity = ReadLightColor(element, "Intensity");
            const float innerAngle = ReadChildFloat(element, "InnerAngle");
            const float outerAngle = ReadChildFloat(element, "OuterAngle");
            if (innerAngle < 0.0f || innerAngle >= outerAngle || outerAngle >= 180.0f)
            {
                throw SceneError(element, "spot angles need 0 <= InnerAngle < OuterAngle < 180 degrees");
            }
            const Vector3f attenuations = ReadLightAttenuations(element);
            scene.CreateLight<SpotLight>(position, direction, intensity,
                glm::radians(innerAngle), glm::radians(outerAngle), attenuations);
        }
        else
        {
            throw SceneError(element, "unsupported light element");
        }
    }

    void ParseLights(const XMLElement* element, Scene& scene)
    {
        CheckAttributes(element, {});
        for (const XMLElement* child = element->FirstChildElement(); child; child = child->NextSiblingElement())
        {
            ParseNestedLight(child, scene);
        }
    }

    Camera ParseNestedCamera(const XMLElement* element, int width, int height)
    {
        CheckValueFields(element, { "Position", "Target", "Up", "NearZ", "FarZ", "Fov" });
        const Vector3f position = ReadChildVector3(element, "Position");
        const Vector3f target = ReadChildVector3(element, "Target");
        const Vector3f up = ReadChildVector3(element, "Up");
        const float nearZ = ReadOptionalChildFloat(element, "NearZ", 0.1f);
        const float farZ = ReadOptionalChildFloat(element, "FarZ", 1000.0f);
        const float fov = ReadOptionalChildFloat(element, "Fov", 45.0f);
        if (nearZ <= 0.0f || farZ <= nearZ || fov <= 0.0f || fov >= 180.0f ||
            glm::length(glm::cross(target - position, up)) <= 1e-6f)
            throw SceneError(element, "invalid camera direction, FOV, or clipping planes");
        Camera camera;
        camera.Initialize(position, target, up, glm::radians(fov), nearZ, farZ, width, height);
        return camera;
    }

    void ParseNestedObjects(const XMLElement* elements, Scene& scene)
    {
        CheckChildren(elements, { "SceneObject" }, false);
        if (!elements->FirstChildElement()) throw SceneError(elements, "expected at least one SceneObject");

        Material* defaultMaterial = nullptr;
        for (const XMLElement* element = elements->FirstChildElement(); element; element = element->NextSiblingElement())
        {
            CheckChildren(element, { "Transform", "Material", "Emissive", "Primitives" });
            const XMLElement* transform = RequireChild(element, "Transform");
            CheckValueFields(transform, { "Position", "Rotation", "Scale" });
            const Vector3f position = transform->FirstChildElement("Position") ?
                ReadChildVector3(transform, "Position") : Vector3f(0.0f);
            const Vector3f rotation = transform->FirstChildElement("Rotation") ?
                ReadChildVector3(transform, "Rotation") : Vector3f(0.0f);
            const float scale = ReadOptionalChildFloat(transform, "Scale", 1.0f);
            if (scale <= 0.0f) throw SceneError(transform, "Scale must be positive");

            Material* material;
            if (element->FirstChildElement("Material"))
            {
                const std::string name = ReadChildText(element, "Material");
                material = scene.GetMaterial(name);
                if (!material)
                {
                    throw SceneError(element, "unknown material '" + name + "'");
                }
            }
            else
            {
                // Older scenes omit material references.
                if (!defaultMaterial)
                {
                    std::string name = "__chapter05_default";
                    while (scene.GetMaterial(name)) name += "_";
                    defaultMaterial = scene.CreateMaterial<LambertMaterial>(name, Color(0.8f));
                }
                material = defaultMaterial;
            }
            SceneObject* object = scene.CreateSceneObject(position, glm::radians(rotation), scale);
            object->SetMaterial(material);
            if (element->FirstChildElement("Emissive"))
            {
                const Color emissive = ReadChildVector3(element, "Emissive");
                if (glm::any(glm::lessThan(emissive, Color(0.0f))))
                {
                    throw SceneError(element->FirstChildElement("Emissive"), "emissive components must be nonnegative");
                }
                object->SetEmissive(emissive);
            }

            const XMLElement* primitives = RequireChild(element, "Primitives");
            CheckChildren(primitives, { "Sphere", "Disk", "Triangle" }, false);
            if (!primitives->FirstChildElement()) throw SceneError(primitives, "expected at least one primitive");
            for (const XMLElement* primitive = primitives->FirstChildElement(); primitive; primitive = primitive->NextSiblingElement())
            {
                const std::string type = primitive->Name();
                if (type == "Triangle")
                {
                    CheckChildren(primitive, { "Vertex" }, false);
                    Vector3f vertices[3];
                    int count = 0;
                    for (const XMLElement* vertex = primitive->FirstChildElement(); vertex; vertex = vertex->NextSiblingElement())
                    {
                        if (count == 3) throw SceneError(primitive, "Triangle needs exactly three Vertex elements");
                        vertices[count++] = ReadElementVector3(vertex);
                    }
                    if (count != 3) throw SceneError(primitive, "Triangle needs exactly three Vertex elements");
                    if (glm::length(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0])) <= 1e-6f)
                        throw SceneError(primitive, "triangle vertices must not be collinear");
                    object->CreatePrimitive<Triangle>(vertices[0], vertices[1], vertices[2]);
                }
                else
                {
                    CheckValueFields(primitive, { "Radius" });
                    const float radius = ReadOptionalChildFloat(primitive, "Radius", 1.0f);
                    if (radius <= 0.0f) throw SceneError(primitive, "Radius must be positive");
                    if (type == "Sphere") object->CreatePrimitive<Sphere>(radius);
                    else object->CreatePrimitive<Disk>(radius);
                }
            }
            if (glm::any(glm::greaterThan(object->GetEmissive(), Color(0.0f))))
            {
                scene.CreateLight<AreaLight>(object);
            }
        }
    }
}

std::unique_ptr<Scene> LoadSceneFromXml(const std::string& path, int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        throw std::runtime_error("viewport dimensions must be positive");
    }

    tinyxml2::XMLDocument document;
    if (document.LoadFile(path.c_str()) != tinyxml2::XML_SUCCESS)
    {
        throw std::runtime_error("cannot load '" + path + "': " + document.ErrorStr());
    }

    const XMLElement* root = document.RootElement();
    if (!root || (std::string(root->Name()) != "scene" && std::string(root->Name()) != "Scene"))
    {
        throw std::runtime_error("'" + path + "' must have a <scene> or <Scene> root element");
    }
    CheckAttributes(root, {});

    std::unique_ptr<Scene> scene(new Scene());
    for (const XMLElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement())
    {
        const std::string type = element->Name();
        if (type == "material")
        {
            ParseMaterial(element, *scene);
        }
        else if (type == "Materials")
        {
            ParseNestedMaterials(element, *scene);
        }
    }

    bool hasCamera = false;
    bool hasObject = false;
    for (const XMLElement* element = root->FirstChildElement(); element; element = element->NextSiblingElement())
    {
        const std::string type = element->Name();
        if (type == "camera" || type == "Camera")
        {
            if (hasCamera)
            {
                throw SceneError(element, "only one camera is allowed");
            }
            scene->SetCamera(type == "Camera" ? ParseNestedCamera(element, width, height) : ParseCamera(element, width, height));
            hasCamera = true;
        }
        else if (type == "object")
        {
            ParseObject(element, *scene);
            hasObject = true;
        }
        else if (type == "SceneObjects")
        {
            ParseNestedObjects(element, *scene);
            hasObject = true;
        }
        else if (type == "light")
        {
            ParseLight(element, *scene);
        }
        else if (type == "Lights")
        {
            ParseLights(element, *scene);
        }
        else if (type == "material" || type == "Materials")
        {
            continue;
        }
        else
        {
            throw SceneError(element, "unsupported scene element");
        }
    }
    if (!hasCamera || !hasObject)
    {
        throw SceneError(root, "scene needs one camera and at least one object");
    }
    return scene;
}

Scene* LoadSceneFromEnvironment(int width, int height)
{
    const char* path = std::getenv("MYRENDER_SCENE");
    if (!path || !*path)
    {
        path = "scenes/scene08.xml";
    }

    try
    {
        return LoadSceneFromXml(path, width, height).release();
    }
    catch (const std::exception& error)
    {
        std::cerr << "Scene load failed: " << error.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
}
