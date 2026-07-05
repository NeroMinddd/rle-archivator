#include <iostream>
#include <string>
#include <fstream>
#include <bitset>

const std::string MAGIC = "ARH";

struct FileHeader {
    uint16_t pathLengh;
    std::string filePath;
    uintmax_t originalSize;
    uintmax_t compressedSize;
};

class Pack {
    private :
        std::string archiveName;
        
        void fileCompress(const std::string& filePath, std::ofstream& archiveFile);

    public:
        Pack(const std::string&  outputhArchiveName);

        void Pack::createArchive(const std::vector<std::string>& filesToPack){
            std::ofstream file(archiveName, std::ios::binary);

            if(!file.is_open()){
                std::cerr << "Error " << std::endl;
                return;
            }

            file.write(MAGIC.c_str(), MAGIC.size());

            uint16_t fileCount = filesToPack.size();

            file.write(reinterpret_cast<const char*>(&fileCount), sizeof(fileCount));

            for(const std::string& filePath : filesToPack){
                fileCompress(filePath, file);
            }

            file.close();
            std::cout << "Archive successfully created! " << std::endl;
        }

        void Pack::fileCompress (std::string& filePath, std::ofstream& archiveFile){
            std::ofstream file(archiveName, std::ios::binary);
            uint16_t originalSize = sizeof(file);
            uint16_t pathLength = static_cast<uint16_t>(filePath.size());

            std::streampos pos = archiveFile.tellp();

            uint8_t compressedSize = 0;

            std::streampos dataStart = archiveFile.tellp();

            for(int i = 0; i < originalSize; ++i){
                if(i < originalSize && archiveFile[i] == archiveFile(i) + 1){

                }
            }

        }

};