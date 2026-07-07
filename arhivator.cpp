#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cstdint>

const std::string MAGIC = "ARH";

class Pack {
private:
    std::string archiveName;
    
    void fileCompress(const std::string& filePath, std::ofstream& archiveFile) {
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if(!file.is_open()){
            std::cerr << "File opening error: " << filePath << std::endl;
            return;
        }

        uintmax_t originalSize = file.tellg();
        file.seekg(0, std::ios::beg);

        uint16_t pathLength = static_cast<uint16_t>(filePath.size());
        archiveFile.write(reinterpret_cast<const char*>(&pathLength), sizeof(pathLength));
        archiveFile.write(filePath.c_str(), pathLength);

        archiveFile.write(reinterpret_cast<const char*>(&originalSize), sizeof(originalSize));

        std::streampos sizePos = archiveFile.tellp();
        uintmax_t compressedSizePlaceholder = 0;
        archiveFile.write(reinterpret_cast<const char*>(&compressedSizePlaceholder), sizeof(compressedSizePlaceholder));

        std::streampos dataPos = archiveFile.tellp();

        char current;
        char next;

        if(file.get(current)){
            uint8_t count = 1;
            
            while(file.get(next)){
                if(current == next && count < 127){
                    ++count;
                }else{
                    uint8_t ctrlByte = count | 0x80;
                    archiveFile.write(reinterpret_cast<const char*>(&ctrlByte), 1);
                    archiveFile.write(&current, 1);

                    current = next;
                    count = 1;
                }
            }
            uint8_t ctrlByte = count | 0x80;
            archiveFile.write(reinterpret_cast<const char*>(&ctrlByte), 1);
            archiveFile.write(&current, 1);
        }

        std::streampos dataEnd = archiveFile.tellp();
        uintmax_t actualCompressedSize = dataEnd - dataPos;
        archiveFile.seekp(sizePos);
        archiveFile.write(reinterpret_cast<const char*>(&actualCompressedSize), sizeof(actualCompressedSize));
        archiveFile.seekp(dataEnd);
        
        file.close();
    }

public:
    Pack(const std::string& outputArchiveName) : archiveName(outputArchiveName) {}

    void createArchive(const std::vector<std::string>& filesToPack){
        std::ofstream file(archiveName, std::ios::binary);

        if(!file.is_open()){
            std::cerr << "Error creating archive file" << std::endl;
            return;
        }

        file.write(MAGIC.c_str(), MAGIC.size());

        uint16_t fileCount = filesToPack.size();
        file.write(reinterpret_cast<const char*>(&fileCount), sizeof(fileCount));

        for(const std::string& filePath : filesToPack){
            fileCompress(filePath, file);
        }

        file.close();
        std::cout << "Archive successfully created!" << std::endl;
    }
};

class Unpack {
private:
    std::string archiveName;

public:
    Unpack(const std::string& inputArchiveName) : archiveName(inputArchiveName) {}

    void extractArchive(){
        std::ifstream file(archiveName, std::ios::binary);
        if(!file.is_open()){
            std::cerr << "File opening error" << std::endl;
            return;
        }

        char magicBuff[3];
        file.read(magicBuff, 3);
        if(std::string(magicBuff, 3) != MAGIC){
            std::cerr << "Error, invalid file type" << std::endl;
            return;
        }

        uint16_t fileCount = 0;
        file.read(reinterpret_cast<char*>(&fileCount), sizeof(fileCount));
        std::cout << "File count in archive: " << fileCount << std::endl;
        
        for(uint16_t i = 0; i < fileCount; ++i){
            uint16_t pathLength = 0;
            file.read(reinterpret_cast<char*>(&pathLength), sizeof(pathLength));

            std::string filePath(pathLength, '\0');
            file.read(&filePath[0], pathLength);

            uintmax_t originalsize = 0;
            uintmax_t compressedSize = 0;
            file.read(reinterpret_cast<char*>(&originalsize), sizeof(originalsize));
            file.read(reinterpret_cast<char*>(&compressedSize), sizeof(compressedSize));

            std::ofstream unpackedFile(filePath, std::ios::binary);
            if(!unpackedFile.is_open()){
                std::cerr << "Error creating unpacked file: " << filePath << std::endl;
                continue;
            }

            uintmax_t bytes_written = 0; 

            while(bytes_written < originalsize){
                uint8_t ctrlByte = 0;
                file.read(reinterpret_cast<char*>(&ctrlByte), 1);
                
                uint8_t count = ctrlByte & 0x7F;

                char dataByte;
                file.read(&dataByte, 1);

                for(uint8_t c = 0; c < count; ++c){
                    unpackedFile.put(dataByte);
                }
                bytes_written += count;
            }

            unpackedFile.close();
            std::cout << "Extracted: " << filePath << std::endl;
        }
        file.close();
        std::cout << "Archive successfully unpacked bradar" << std::endl;
    }
};

int main() {
    std::vector<std::string> files = {"test1.txt", "test2.txt"};
    std::string archiveName = "my_archive.arh";

    Pack archiver(archiveName);
    archiver.createArchive(files);

    Unpack decompressor(archiveName);
    decompressor.extractArchive();

    return 0;
}
