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
        AutoBuffer(size_t capacity);
        AutoBuffer(const char *data, size_t size);

        void reset();
        void reset(size_t capacity);
        void reset(const char *data, size_t size);

        bool fill(size_t offset, const char *data, size_t size);
        void resize(size_t size = 0);

        char* data();
        size_t size() const;
        size_t capacity() const;

        bool loadFromFile(const std::string& file);
        bool saveToFile(const std::string& file) const;

    private:
        static void free(char *p);

    private:
        std::shared_ptr<char> m_sptrData;
        size_t m_size;
        size_t m_capacity;
    };
}



#endif
