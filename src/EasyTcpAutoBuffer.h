#ifndef EASYTCPAUTOBUFFER_H
#define EASYTCPAUTOBUFFER_H

#include <string>
#include <memory>

namespace EasyTCP
{
    class AutoBuffer
    {
    public:
        AutoBuffer();
        AutoBuffer(size_t size);
        AutoBuffer(const char *data, size_t size);

        void reset();
        bool reset(size_t size);
        bool reset(const char *data, size_t size);

        char* data();
        size_t size();

        bool loadFromFile(const std::string& file);
        bool saveToFile(const std::string& file);

    private:
        static void free(char *p);

    private:
        std::shared_ptr<char> m_sptrData;
        size_t m_size;
    };
}



#endif
