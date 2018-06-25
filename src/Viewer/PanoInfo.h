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
	std::string annotations;
	float verticalCorrection = 0;
	float horizontalCorrection = 0;
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
			if (panosArray[i].HasMember("id")) {
				pano.id = panosArray[i]["id"].GetString();
			}
			if (panosArray[i].HasMember("name")) {
				pano.displayName = panosArray[i]["name"].GetString();
			}
			if (panosArray[i].HasMember("left")) {
				pano.leftAddress = baseImageURL + '/' + panosArray[i]["left"].GetString();
			}
			if (panosArray[i].HasMember("right")) {
				pano.rightAddress = baseImageURL + '/' + panosArray[i]["right"].GetString();
			}
			if (panosArray[i].HasMember("thumb")) {
				pano.thumbAddress = baseImageURL + '/' + panosArray[i]["thumb"].GetString();
			}
			if (panosArray[i].HasMember("annotations")) {
				pano.annotations = baseImageURL + '/' + panosArray[i]["annotations"].GetString();
			}
			if (panosArray[i].HasMember("v-correction")) {
				pano.verticalCorrection = panosArray[i]["v-correction"].GetFloat();
			}
			if (panosArray[i].HasMember("h-correction")) {
				pano.horizontalCorrection = panosArray[i]["h-correction"].GetFloat();
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