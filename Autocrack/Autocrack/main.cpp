#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cctype>
#include <sstream>
#include <windows.h>
#include "RTParser.hpp"

std::string current_version;


/*******************   KEYAUTH VARIABLES   *******************/

// IMPORTANT NOTE:
// If you are using own KeyAuth, you gotta name the application as "Premium Proxy" to make it work.

std::string original_ownerid = "fRfa2mUrtR";
std::string original_secret = "dcce38172ead56bccb8fba908deb3b7baac048f0383c28f60d76f515f77b950a";
std::string new_ownerid = "KUJxDZyPai";
std::string new_secret = "e45b151a42f87533b967fdb497d48e874b4668085f0600fa1b69a102c070ddcd";

/*************************************************************/


bool edit_string_to(std::string filePath, std::string oldText, std::string newText, int stopAfterFound = -1) {
	if (newText.size() > oldText.size()) return false;

	std::ifstream in(filePath, std::ios::binary);
	if (!in) return false;

	std::vector<char> buffer((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	in.close();

	bool replaced = false;
	int found = 0;
	for (size_t i = 0; i + oldText.size() <= buffer.size(); ++i) {
		if (std::memcmp(&buffer[i], oldText.data(), oldText.size()) == 0) {

			if (stopAfterFound != -1 && ++found >= stopAfterFound)
				break;

			std::memcpy(&buffer[i], newText.data(), newText.size());
			for (size_t j = newText.size(); j < oldText.size(); ++j)
				buffer[i + j] = '\0';
			replaced = true;
		}
	}

	if (!found && oldText == "fRfa2mUrtR" && std::stof(current_version.substr(1)) >= 3.8753f)
		in.close(),
		printf("\n\033[31mThe file is corrupted. (No OwnerID Found)\n"),
		printf("\nYou gotta unpack the file again, unless you saved a copy of the unpacked file, and then re-run this Autocrack.\033[0m\n"),
		Sleep(-1);

	if (found <= 3 && found > 0 && std::stof(current_version.substr(1)) >= 3.8753f) { // Accidental re-run perhaps
		in.close();
		return false;
	}

	if (replaced) {
		std::ofstream out(filePath, std::ios::binary | std::ios::trunc);
		if (!out) return false;
		out.write(buffer.data(), buffer.size());
		out.close();
	}

	return replaced;
}
static inline uint8_t hexCharToVal(char c) {
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
	if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
	throw std::invalid_argument("invalid hex char");
}

std::vector<uint8_t> parseHexBytes(std::string s) {
	std::string t;
	for (char c : s) if (!isspace(static_cast<unsigned char>(c))) t.push_back(c);
	if (t.size() % 2 != 0) throw std::invalid_argument("hex string length must be even");

	std::vector<uint8_t> out;
	for (size_t i = 0; i < t.size(); i += 2) {
		uint8_t b = (hexCharToVal(t[i]) << 4) | hexCharToVal(t[i + 1]);
		out.push_back(b);
	}
	return out;
}

bool patchBytesFromOffset(std::string path, size_t offset, std::string hexBytes) {
	std::vector<uint8_t> patch;
	try {
		patch = parseHexBytes(hexBytes);
	}
	catch (...) {
		return false;
	}

	std::ifstream in(path, std::ios::binary | std::ios::ate);
	if (!in) return false;

	std::streamsize sz = in.tellg();
	in.seekg(0, std::ios::beg);
	std::vector<uint8_t> buf(static_cast<size_t>(sz));
	if (!in.read(reinterpret_cast<char*>(buf.data()), sz)) return false;
	in.close();

	if (offset + patch.size() > buf.size()) buf.resize(offset + patch.size(), 0x00);

	for (size_t i = 0; i < patch.size(); ++i) buf[offset + i] = patch[i];

	std::ofstream out(path, std::ios::binary | std::ios::trunc);
	if (!out) return false;
	out.write(reinterpret_cast<const char*>(buf.data()), buf.size());
	return out.good();
}

void patch(std::string path, size_t offset, std::string bytes) {
	if (patchBytesFromOffset(path, offset, bytes)) {
		printf("Successfully Patched Offset: %d !\n", (int)offset);
		return;
	}
	printf("Failed to patch. (Close the proxy if it's open)\n");
	Sleep(-1);
}


void edit_string(std::string filePath, std::string oldText, std::string newText, int stopAfterFound = -1) {
	if (!edit_string_to(filePath, oldText, newText, stopAfterFound)) {
		printf("Failed to replace \"%s\"  ->  \"%s\"  (Not found. It was probably replaced already)\n", oldText.c_str(), newText.c_str());
		return;
	}
	printf("Successfully replaced \"%s\"  ->  \"%s\"\n", oldText.c_str(), newText.c_str());
}

struct PatchData {
	std::string ProxyVersion;
	int sizeKB[2]{};
	std::vector<std::pair<int, std::string>> addresses;
};
std::vector<PatchData> patchData;

void LoadPatchData(std::string content) {

	std::string f_content;

	{
		std::istringstream iss(content);
		std::string line;

		std::string curr_version;

		while (std::getline(iss, line)) {
			if (line.find("AddVersionSupport|") != -1) {
				RTParser parser = RTParser::parse(line);
				auto data = parser.find("AddVersionSupport")->m_values;
				PatchData p;
				p.ProxyVersion = data[0];
				p.sizeKB[0] = stoi(data[1]);
				p.sizeKB[1] = stoi(data[2]);
				patchData.push_back(p);
			}
			if (line.find("[START_") != -1) {
				curr_version = line.substr(line.find("[START_") + 7);
				if (curr_version.find("]") != -1) curr_version.erase(curr_version.begin() + curr_version.find("]"), curr_version.end());

				bool found = false;
				for (PatchData p : patchData)
					if (p.ProxyVersion == curr_version) {
						found = true;
						break;
					}
				if (!found) {
					PatchData p;
					p.ProxyVersion = curr_version;
					patchData.push_back(p);
				}
			}
			f_content += line + '\n';
		}
	}
	{
		int i = 0;
		while (i < patchData.size()) {
			std::string version = patchData[i].ProxyVersion;
			std::string data = f_content.substr(f_content.find("[START_" + version + "]\n") + ("[START_" + version + "]\n").length() - 1);
			if (data.find("[END_" + version + "]") != -1) data.erase(data.begin() + data.find("[END_" + version + "]"), data.end());

			std::string line;
			std::istringstream iss(data);

			while (std::getline(iss, line)) {
				if (line.rfind("0x", 0) == 0) {
					uintptr_t addr = stoull(line.substr(0, line.find("  ")), nullptr, 0);
					std::string bytes = line.substr(line.find("  ") + 2);

					patchData[i].addresses.push_back(
						std::make_pair((int)addr, bytes)
					);
				}
			}
			i++;
		}
	}
}

bool FetchPatchData() {
	printf("Fetching Data From Web...\r");

	//printf("==========[CURL RESPONSE]==========\n");
	system("curl https://raw.githubusercontent.com/JoakimTheCoder/SilviozasAutoCrack/refs/heads/main/PatchData -s -o PatchData.txt");
	//printf("==========[END CURL RESPONSE]==========\n");

	std::ifstream file("PatchData.txt");
	if (!file.is_open())
		return false;

	std::string content, line;
	while (std::getline(file, line))
		content += line + '\n';
	LoadPatchData(content);
	return true;
}

PatchData Initialize(int sizeKB) {
	for (PatchData p : patchData) {
		if (p.sizeKB[0] == sizeKB ||
			p.sizeKB[1] == sizeKB)
			return p;
	}
	return {};
}

int main() {

	printf("Silviozas Auto Crack Made By joakimmer1k0. Any problems or need help? Message me in Discord.\n\n");

	if (!FetchPatchData()) {
		printf("Couldn't fetch Patch Data for AutoCrack (check your internet and try again.)\n");
		Sleep(-1);
	}
	printf("Currently Supported Proxy Versions: [");
	for (PatchData p : patchData) printf("%s (%dkb) ", p.ProxyVersion.c_str(), p.sizeKB[1]);
	printf("]\n");

	//for (PatchData p : patchData) {
	//	printf("START=======[VERSION: %s]=======\n", p.ProxyVersion.c_str());
	//	for (auto a : p.addresses)
	//		printf("[Address: %d]  [Bytes: %s]\n", a.first, a.second.c_str());
	//	printf("END=======[VERSION: %s]=======\n", p.ProxyVersion.c_str());
	//}

	if ((int)new_ownerid.length() != 10 || (int)new_secret.length() != 64) {
		printf("[ERROR]: new_ownerid and new_secret lengths has to be 10 and 64.\n");
		Sleep(-1);
	}

	std::string path = R"(C:\Users\Public\Proxy_Stuff\Silviozas Premium Proxy.exe)";
	std::ifstream in(path, std::ios::binary);
	if (!in || !in.is_open()) {
		printf("Proxy File Not Found. Path Should Be At: [%s]\n", path.c_str());
		Sleep(-1);
	}
	in.seekg(0, std::ios::end);

	int sizeKB = (int)in.tellg() / 1024;

	if (sizeKB < 5000)
	{
		printf("[ERROR]: File Isn't Unpacked. Use XVolkolak to Unpack It. Size (kb): %d\n", sizeKB);
		printf("If You Already Unpacked The File, Rename It to \"Silviozas Premium Proxy\"\n");
		Sleep(-1);
	}
	//printf("KB: %d\n", sizeKB);
	in.seekg(0, std::ios::beg);

	PatchData data = Initialize(sizeKB);

	if (data.addresses.empty()) {
		printf("This Proxy Version Isn't Supported. If There's A New Update, Tell Joakim.\n");
		Sleep(-1);
		return 0;
	}

	printf("Current Proxy Version Is: %s\n\n", data.ProxyVersion.c_str());
	
	current_version = data.ProxyVersion;

	edit_string(path, original_ownerid, new_ownerid, stof(data.ProxyVersion.substr(1)) >= 3.8753f ? 40 : -1);

	edit_string(path, original_secret, new_secret);

	printf("\nStarting patch...\n\n");
	for (auto addr : data.addresses)
		patch(path, addr.first, addr.second);

	printf("\nFinished Patching! (Proxy is ready to use)\n");

	Sleep(-1);
	return 0;
}
