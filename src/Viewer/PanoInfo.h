#ifndef _PANOINFO_H
#define _PANOINFO_H
#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <string>
#include <vector>


struct PanoInfo
{
	std::string id;
	std::string displayName;
	std::string leftAddress;
	std::string rightAddress;
	std::string thumbAddress;
};

static std::vector<PanoInfo> parsePanoInfoFile(std::string jsonFileRawText, std::string baseImageURL) {
	std::vector<PanoInfo> panoInfoList;
	rapidjson::Document jsonDocument;
	jsonDocument.Parse(jsonFileRawText.c_str());
	if (!jsonDocument.HasParseError() && jsonDocument.HasMember("panos"))
	{
		rapidjson::Value& panosArray = jsonDocument["panos"];
		// Iterate through the array of panos
		for (unsigned int i = 0; i < panosArray.Size(); ++i)
		{
			PanoInfo pano;
			if (panosArray[i].HasMember("id")
				&& panosArray[i].HasMember("name")
				&& panosArray[i].HasMember("left")
				&& panosArray[i].HasMember("right")
				&& panosArray[i].HasMember("thumb"))
			{
				pano.id = panosArray[i]["id"].GetString();
				pano.displayName = panosArray[i]["name"].GetString();
				pano.leftAddress = baseImageURL + '/' + panosArray[i]["left"].GetString();
				pano.rightAddress = baseImageURL + '/' + panosArray[i]["right"].GetString();
				pano.thumbAddress = baseImageURL + '/' + panosArray[i]["thumb"].GetString();
			}
			else
			{
				printf("Incomplete data for pano no. %d\n", i);
			}
			panoInfoList.push_back(pano);
		}
	}
	else
	{
		printf("Could not read the pano JSON file; %s", rapidjson::GetParseError_En(jsonDocument.GetParseError()));
	}
	return panoInfoList;
}
#endif //_PANOINFO_H