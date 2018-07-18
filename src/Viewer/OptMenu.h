#include <stdio.h>
#include <iostream>
#include <string>
#include "Shared.h"

#include "KinectControl.h"
#include "RemoteServent.h"

const char* LoadPath()
{
	const char *path = NULL;

	fprintf(stderr, "\n\n\n---- If loading from a filepath, prefix with File:\n");
	fprintf(stderr, "---- EX: File:Z:\\Some\\File\\Path\n");

	std::string stringpath;
	std::cin >> stringpath;
	if (stringpath.length() > 0)
		strcpy_s((char*)path, stringpath.length(), stringpath.c_str());

	return path;
}

void SetBorderless(int &width, int &height)
{
	fprintf(stderr, "\n\n\n---- Enter Width then Height\n");
	width = scanf_s("%d");
	height = scanf_s("%d");
}

void SetRemoteOptions(RemoteServent *remote)
{
	while (remote != NULL) {
		fprintf(stderr, "\n\nRemote was already configured, reconfigure? [Y\N] \n");
		int choice = scanf_s("%d");
		if (choice == 78 || choice == 110)
			return;
		if (choice == 89 || choice == 121)
			break;
		fprintf(stderr, "Invalid choice given.\n");
	}

	delete remote;
	bool configured = false;
	while (!configured) {

		fprintf(stderr, "Enter IP Address of Remote Host:\n");
		std::string IP;
		std::getline(std::cin, IP);
		if (IP.length() < 1) {
			fprintf(stderr, "Invalid string length\n");
			continue;
		}

		fprintf(stderr, "Enter Port of Remote Host:\n");
		int port = scanf_s("%d");
		if (port < 1) {
			fprintf(stderr, "Invalid port provided\n");
			continue;
		}

		fprintf(stderr, "Enter a name for this machine:\n");
		std::string name;
		std::getline(std::cin, name);
		if (name.length() < 1) {
			fprintf(stderr, "No name provided, defaulting\n");
			name = "A Computer With No Name";
		}

		fprintf(stderr, "Is this a distributed display? [Y\N] \n");
		std::string position = "";
		int yesno = scanf_s("%d");
		if (yesno == 89 || yesno == 121) {
			bool positionSet = false;
			while (!positionSet) {
				fprintf(stderr, "Please specify the position of this machine in relation to the host\n");
				fprintf(stderr, "Up/Above - u, Down/Below - d, Left - l, Right - r, UpperLeft - ul, LowerLeft - dl, etc\n");
				fprintf(stderr, "Do not include a dash or hyphen with your response\n");
				std::getline(std::cin, position);
				if (position.length() > 2) {
					fprintf(stderr, "Received %s, not a valid selection\n", position);
					continue;
				}
			}
			remote = new RemoteServent(IP.c_str(), port, name.c_str(), position);
			return;
		}

		configured = true;
		remote = new RemoteServent(IP.c_str(), port, name.c_str());
	}
}

void SetSyncViewOptions(RemoteServent *remote)
{
	while (remote != NULL) {
		fprintf(stderr, "\n\nRemote was already configured, reconfigure? [Y\N] \n");
		int choice = scanf_s("%d");
		if (choice == 78 || choice == 110)
			return;
		if (choice == 89 || choice == 121)
			break;
		fprintf(stderr, "Invalid choice given.\n");
	}

	delete remote;
	bool configured = false;
	while (!configured) {

		fprintf(stderr, "Enter IP Address of This Host:\n");
		std::string IP;
		std::getline(std::cin, IP);
		if (IP.length() < 1) {
			fprintf(stderr, "Invalid string length\n");
			continue;
		}

		fprintf(stderr, "Enter Port to Use:\n");
		int port = scanf_s("%d");
		if (port < 1) {
			fprintf(stderr, "Invalid port provided\n");
			continue;
		}

		fprintf(stderr, "Enter a name for this machine:\n");
		std::string name;
		std::getline(std::cin, name);
		if (name.length() < 1) {
			fprintf(stderr, "No name provided, defaulting\n");
			name = "A Computer With No Name";
		}

		fprintf(stderr, "Is this a distributed display? [Y\N] \n");
		std::string position = "";
		int yesno = scanf_s("%d");
		if (yesno == 89 || yesno == 121) {

			remote = new RemoteServent(IP.c_str(), port, name.c_str(), true, true);
			return;
		}

		configured = true;
		remote = new RemoteServent(IP.c_str(), port, name.c_str(), true, false);
	}
}


const char* Menu(int argc, char **argv, bool &stereo, bool &fullscreen, bool &fivepanel, bool &borderless, int &width, int &height, KinectControl *kinect, RemoteServent *remote)
{
	bool menuOpen = true;
	const char *path = NULL;
	while (menuOpen) {
		fprintf(stderr, "------------------------------\n");
		fprintf(stderr, "---- If a commandline flag exists for a choice, it will be in parens\n");
		fprintf(stderr, "1) Provide Filepath or URL to JSON\n");
		fprintf(stderr, "2) Enable Fullscreen (-f)\n");
		fprintf(stderr, "3) Enable FivePanel mode (-5)\n");
		fprintf(stderr, "4) Enable Stereo Mode (-s)\n");
		fprintf(stderr, "5) Enable Borderless Mode(-b <width> <height>)\n");
		fprintf(stderr, "6) Enable Kinect (-k)");
		fprintf(stderr, "7) Enable Remote (-r <IP> <Port> [name [u|l|r|d|ul|ur|dr|dl]])\n");
		fprintf(stderr, "8) Enable Synchronized Viewing Host (-sv <IP> <Port> [name [-d]]\n");
		fprintf(stderr, "9) Exit this menu with flags and info set\n");
		int choice = 0;
		scanf_s("%d", &choice);
		switch (choice) {
		case 9:
			menuOpen = false;
			break;
		case 1:
			path = LoadPath();
			break;
		case 2:
			fullscreen = !fullscreen;
			fprintf(stderr, "Fullscreen toggled to %d\n", fullscreen);
			break;
		case 3:
			fivepanel = !fivepanel;
			fprintf(stderr, "FivePanel mode toggled to %d\n", fivepanel);
			break;
		case 4:
			stereo = !stereo;
			fprintf(stderr, "Stereoscopic mode toggled to %d\n", stereo);
			break;
		case 5:
			SetBorderless(width, height);
			break;
		case 6:
#ifdef KINECT
			kinect = new KinectControl;
#else
			fprintf(stderr, "This copy of TileViewer was compiled without Kinect support\n");
#endif
			break;
		case 7:
			SetRemoteOptions(remote);
			break;
		case 8:
			SetSyncViewOptions(remote);
			break;
		default:
			fprintf(stderr, "Unrecognized choice\n");
			break;
		}
	}
	return path;
}