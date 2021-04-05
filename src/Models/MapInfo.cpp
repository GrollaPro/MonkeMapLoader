#include "Models/MapInfo.hpp"

#include "libzip/shared/zip.h"
#include "beatsaber-hook/shared/utils/utils.h"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp"
#include "Utils/FileUtils.hpp"

std::string toLower(std::string aString)
{
    std::string result = "";

    for (auto c : aString)
    {
        result += tolower(c);
    }

    return result;
}

extern Logger& getLogger();
namespace MapLoader
{
    void MapInfo::Load()
    {
        if (!fileexists(filePath)) return;
        int err = 0;
        zip *z = zip_open(filePath.c_str(), 0, &err);
        
        struct zip_stat st;
        zip_stat_init(&st);
        zip_stat(z, "package.json", 0, &st);
        zip_file* f = zip_fopen(z, st.name, 0);
        char* package = new char[st.size];
        zip_fread(f, package, st.size);
        zip_fclose(f);
        
        std::string json = "";
        for (int i = 0; i < st.size; i++)
        {
            json += package[i];
        }
        
        rapidjson::Document doc;
        doc.Parse(json.c_str());
        packageInfo = new MapPackageInfo(doc);
        delete[](package);
        zip_close(z);
    }

    Il2CppObject* MapInfo::LoadCubeMap()
    {
        if (packageInfo->previewCubeMap) return packageInfo->previewCubeMap;
        using LoadImage = function_ptr_t<unsigned int, Il2CppObject*, Array<uint8_t>*, bool>;
        static LoadImage loadImage = reinterpret_cast<LoadImage>(il2cpp_functions::resolve_icall("UnityEngine.ImageConversion::LoadImage"));

        int err = 0;
        zip *z = zip_open(filePath.c_str(), 0, &err);
        
        struct zip_stat st;
        zip_stat_init(&st);
        zip_stat(z, packageInfo->config.cubeMapImagePath.c_str(), 0, &st);

        zip_file* f = zip_fopen(z, st.name, 0);
        char* cubeMap = new char[st.size];
        zip_fread(f, cubeMap, st.size);
        zip_fclose(f);
        zip_close(z);
        std::vector<uint8_t> byteVector;
        for (int i = 0; i < st.size; i++)
        {
            byteVector.push_back((uint8_t)cubeMap[i]);
        }
        Array<uint8_t>* byteArray = il2cpp_utils::vectorToArray(byteVector);
        static Il2CppClass* textureKlass = il2cpp_utils::GetClassFromName("UnityEngine", "Texture2D");

        Il2CppObject* texture = *il2cpp_utils::New(textureKlass, 0, 0, 4, false);
        loadImage(texture, byteArray, false);
        delete[](cubeMap);
        packageInfo->previewCubeMap = texture;
        return packageInfo->previewCubeMap;
    }

    bool MapInfo::operator<(const MapInfo& second) const
    {
        std::string firstName = toLower(this->packageInfo->descriptor.mapName);
        std::string secondName = toLower(second.packageInfo->descriptor.mapName);
        return firstName < secondName;
    }

}