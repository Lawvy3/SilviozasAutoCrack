#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cctype>
#include <sstream>
#include <windows.h>

std::string current_version;

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
			//printf("Found: %d\n", found);
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

/*******************   KEYAUTH VARIABLES   *******************/

// IMPORTANT NOTE:
// If you are using own KeyAuth, you gotta name the application as "Premium Proxy" to make it work.

std::string original_ownerid = "fRfa2mUrtR";
std::string original_secret = "dcce38172ead56bccb8fba908deb3b7baac048f0383c28f60d76f515f77b950a";
std::string new_ownerid = "KUJxDZyPai";
std::string new_secret = "e45b151a42f87533b967fdb497d48e874b4668085f0600fa1b69a102c070ddcd";

/*************************************************************/


std::vector<std::pair<int, std::string>> addresses;

void GetAdresses(std::string pVersion) {
	current_version = pVersion;

	if (pVersion == "V3.87461")
		addresses =
	{
		std::make_pair(0x624077, "90 90 89 05 B9 28 2A 00 EB 0E 8B 05 A1 28 2A 00"),
		std::make_pair(0x624087, "90 90 89 05 99 28 2A 00 48 8D 15 06 DD 1D 00 48"),
		std::make_pair(0x624138, "90 90 89 05 F8 27 2A 00 EB 0E 8B 05 E0 27 2A 00"),
		std::make_pair(0x624148, "90 90 89 05 D8 27 2A 00 48 8D 84 24 50 02 00 00"),
		std::make_pair(0x5F1FD7, "90 90 89 05 59 49 2D 00 EB 0E 8B 05 41 49 2D 00"),
		std::make_pair(0x5F1FE7, "90 90 89 05 39 49 2D 00 48 8D 15 5E 68 20 00 48"),
		std::make_pair(0x5F2098, "90 90 89 05 98 48 2D 00 EB 0E 8B 05 80 48 2D 00"),
		std::make_pair(0x5F20A8, "90 90 89 05 78 48 2D 00 E8 CB F8 02 00 48 8D 15"),
		std::make_pair(0x6219AE, "75 05 E9 EB 23 00 00 48 8D 84 24 10 12 00 00 48"),
		std::make_pair(0x5EB960, "75 1A 8B 05 C0 AF 2D 00 FF C0 89 05 B8 AF 2D 00"),
		std::make_pair(0x5EBA12, "75 1A 8B 05 0E AF 2D 00 FF C0 89 05 06 AF 2D 00"),
		std::make_pair(0x59B3D0, "75 1A 8B 05 50 B5 32 00 FF C0 89 05 48 B5 32 00"),
		std::make_pair(0x59B482, "75 1A 8B 05 9E B4 32 00 FF C0 89 05 96 B4 32 00"),
		std::make_pair(0x406750, "75 1A 8B 05 D0 01 4C 00 FF C0 89 05 C8 01 4C 00"),
		std::make_pair(0x406802, "75 1A 8B 05 1E 01 4C 00 FF C0 89 05 16 01 4C 00"),
		std::make_pair(0x496D0B, "75 0E C6 05 FF FB 42 00 01 B0 01 E9 DB 11 00 00"),
		std::make_pair(0x40AC1C, "75 05 E9 2F 12 00 00 48 8B 84 24 10 1F 00 00 0F"),
		std::make_pair(0x5EFE2C, "75 05 E9 2F 12 00 00 48 8B 84 24 10 1F 00 00 0F"),
		std::make_pair(0x59F89C, "75 05 E9 2F 12 00 00 48 8B 84 24 10 1F 00 00 0F"),
		std::make_pair(0x40A5B7, "75 05 E9 94 18 00 00 48 8B 84 24 10 1F 00 00 0F"),
		std::make_pair(0x5EF7C7, "75 05 E9 94 18 00 00 48 8B 84 24 10 1F 00 00 0F"),
		std::make_pair(0x59F237, "75 05 E9 94 18 00 00 48 8B 84 24 10 1F 00 00 0F"),
		std::make_pair(0x40114E, "75 05 E9 C8 62 00 00 48 8B 84 24 C0 42 00 00 0F"),
		std::make_pair(0x595DCE, "75 05 E9 C8 62 00 00 48 8B 84 24 C0 42 00 00 0F"),
		std::make_pair(0x5E635E, "75 05 E9 C8 62 00 00 48 8B 84 24 C0 42 00 00 0F"),
		std::make_pair(0x496C4A, "0F 85 CB 00 00 00 48 8B 84 24 70 0A 00 00 0F B6"),
		std::make_pair(0x4C9DBD, "C6 84 24 60 41 01 00 00 48 8D 84 24 80 28 01 00") // alert
	};

	else if (pVersion == "V3.8753")
		addresses =
	{
		std::make_pair(0x4094C4, "75 05 E9 11 63 00 00 48 8B 84 24 20 43 00 00 0F"),
		std::make_pair(0x40EB0F, "75 1A 8B 05 ED AF 56 00 FF C0 89 05 E5 AF 56 00"),
		std::make_pair(0x4129DD, "75 05 E9 78 1A 00 00 48 8B 84 24 40 20 00 00 0F"),
		std::make_pair(0x413226, "74 05 E9 2F 12 00 00 48 8B 84 24 40 20 00 00 0F"),
		std::make_pair(0x58C974, "75 05 E9 11 63 00 00 48 8B 84 24 20 43 00 00 0F"),
		std::make_pair(0x591FBF, "75 1A 8B 05 3D 7B 3E 00 FF C0 89 05 35 7B 3E 00"),
		std::make_pair(0x595E8D, "75 05 E9 78 1A 00 00 48 8B 84 24 40 20 00 00 0F"),
		std::make_pair(0x5966D6, "74 05 E9 2F 12 00 00 48 8B 84 24 40 20 00 00 0F"),
		std::make_pair(0x608B84, "75 05 E9 11 63 00 00 48 8B 84 24 20 43 00 00 0F"),
		std::make_pair(0x60E1CF, "75 1A 8B 05 2D B9 36 00 FF C0 89 05 25 B9 36 00"),
		std::make_pair(0x61209D, "75 05 E9 78 1A 00 00 48 8B 84 24 40 20 00 00 0F"),
		std::make_pair(0x6128E6, "74 05 E9 2F 12 00 00 48 8B 84 24 40 20 00 00 0F"),
		std::make_pair(0x659AF4, "75 05 E9 11 63 00 00 48 8B 84 24 20 43 00 00 0F"),
		std::make_pair(0x65F13F, "75 1A 8B 05 BD A9 31 00 FF C0 89 05 B5 A9 31 00"),
		std::make_pair(0x66300D, "75 05 E9 78 1A 00 00 48 8B 84 24 40 20 00 00 0F"),
		std::make_pair(0x663856, "74 05 E9 2F 12 00 00 48 8B 84 24 40 20 00 00 0F"),
		std::make_pair(0x4D4645, "C6 84 24 E0 45 01 00 00 48 8D 84 24 20 2D 01 00") // alert
	};

	else if (pVersion == "V3.87542")
		addresses =
	{
		std::make_pair(0x6643D1, "75 1A 8B 05 2B B7 31 00 FF C0 89 05 23 B7 31 00"),
		std::make_pair(0x66829D, "75 05 E9 78 1A 00 00 48 8B 84 24 40 20 00 00 0F"),
		std::make_pair(0x65ED86, "75 05 E9 11 63 00 00 48 8B 84 24 80 43 00 00 0F"),
		std::make_pair(0x613331, "75 1A 8B 05 CB C7 36 00 FF C0 89 05 C3 C7 36 00"),
		std::make_pair(0x60DCE6, "75 05 E9 11 63 00 00 48 8B 84 24 80 43 00 00 0F"),
		std::make_pair(0x6171FD, "75 05 E9 78 1A 00 00 48 8B 84 24 40 20 00 00 0F"),
		std::make_pair(0x5967B1, "75 1A 8B 05 4B 93 3E 00 FF C0 89 05 43 93 3E 00"),
		std::make_pair(0x591166, "75 05 E9 11 63 00 00 48 8B 84 24 80 43 00 00 0F"),
		std::make_pair(0x59A67D, "75 05 E9 78 1A 00 00 48 8B 84 24 40 20 00 00 0F"),
		std::make_pair(0x4100F1, "75 1A 8B 05 0B FA 56 00 FF C0 89 05 03 FA 56 00"),
		std::make_pair(0x40AAA6, "75 05 E9 11 63 00 00 48 8B 84 24 80 43 00 00 0F"),
		std::make_pair(0x413FBD, "75 05 E9 78 1A 00 00 48 8B 84 24 40 20 00 00 0F"),
		std::make_pair(0x4D6C10, "C6 84 24 40 48 01 00 00 48 8D 84 24 00 2F 01 00") // alert
	};
}

int main() {
	/*   Works with V3.87461. (If new version is out, tell joakimmer1k0)    */

	printf("Silviozas Auto Crack Made By joakimmer1k0. Any problems or need help? Message me in Discord.\n\n");

	if ((int)new_ownerid.length() != 10 || (int)new_secret.length() != 64) {
		printf("[ERROR]: new_ownerid and new_secret lengths has to be 10 and 64.\n");
		while (true)
			Sleep(1);
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

	switch (sizeKB) {

	case 9640:
		GetAdresses("V3.8753");
		break;

	case 10383:
	case 10382:
		GetAdresses("V3.8753");
		break;

	case 10405:
	case 10406:
		GetAdresses("V3.87542");
		break;

	default:
		printf("This Proxy Version Isn't Supported. If There's A New Update, Tell Joakim.\n");
		Sleep(-1);
	}

	printf("Current Proxy Version Is: %s\n\n", current_version.c_str());

	printf("Starting patch...\n\n");
	edit_string(path, original_ownerid, new_ownerid,
		(current_version == "V3.8753" ||
		current_version == "V3.87542")
		? 40 : -1);

	edit_string(path, original_secret, new_secret);

	for (auto addr : addresses)
		patch(path, addr.first, addr.second);

	printf("\nFinished Patching! (Proxy is ready to use)\n");

	Sleep(-1);
	return 0;
}

