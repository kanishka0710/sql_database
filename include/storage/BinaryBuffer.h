//
// Created by kanis on 4/23/2024.
//

#ifndef ECE141DB_BINARYBUFFER_H
#define ECE141DB_BINARYBUFFER_H

#include <string>
#include <vector>

namespace ECE141 {


    class BinaryBuffer {
    public:
        BinaryBuffer() = default;

        template<typename T>
        bool write(const T &data) {
            const size_t dataSize = sizeof(T);
            const char *binaryData = reinterpret_cast<const char *>(&data);

            for (size_t i = 0; i < dataSize; ++i)
                buffer.push_back(binaryData[i]);

            return true;
        }

        bool writeString(const std::string &data) {
            for (auto character: data)
                buffer.push_back(character);

            buffer.push_back('\0');
            return true;
        }

        template<typename T>
        T read() {
            const size_t dataSize = sizeof(T);
            T data{};
            char *binaryData = reinterpret_cast<char *>(&data);

            for (size_t i = 0; i < dataSize; ++i)
                binaryData[i] = buffer[readPointer + i]; //TODO: Seg fault

            readPointer += dataSize;
            return data;
        }

        std::string readString() {
            std::string data(buffer.data() + readPointer);
            readPointer += data.size() + 1;
            return data;
        }

        size_t getSize() { return buffer.size(); }

        std::vector<char> &getBuffer() { return buffer; };

        size_t getReadPointer() {return readPointer;}

    private:
        std::vector<char> buffer;
        size_t readPointer = 0;

    };

}

#endif //ECE141DB_BINARYBUFFER_H
