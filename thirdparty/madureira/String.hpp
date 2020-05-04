#ifndef STRING_H
#define STRING_H

#include <string.h>
#include <string>

class String
{
private:
    unsigned int m_Length;
    char* m_Buffer;

public:
    String()
    {
        m_Length = 0;
        m_Buffer = new char[0];
    }

    String(const char* cString)
    {
        if (cString)
        {
            unsigned int n = 0;
            unsigned int i = 0;
            while (cString[n] != '\0') n++;
            m_Buffer = new char[n + 1];
            for (i; i < n; i++) m_Buffer[i] = cString[i];
            m_Buffer[n] = '\0';
            m_Length = n;
        }
        else
        {
            m_Length = 0;
            m_Buffer = new char[0];
        }
    }

    String(const String& string)
    {
        int i = 0;
        int len = string.size();
        m_Buffer = new char[len + 1];
        for (i; i < len; i++) m_Buffer[i] = string[i];
        m_Length = len;
        m_Buffer[len] = '\0';
    }

    ~String()
    {
        m_Length = 0;
        delete[] m_Buffer;
    }

    int size() const
    {
        return m_Length;
    }

    const char* c_str() const
    {
        return m_Buffer;
    }

    bool equals(String& other) const
    {
        if (m_Length != other.size()) return false;
        unsigned int n = 0;
        while ((n < m_Length) && (m_Buffer[n] == other[n])) n++;
        return (n == m_Length);
    }

    bool equals(const char* other) const
    {
        String string(other);
        return this->equals(string);
    }

    int index(char c) const
    {
        unsigned int i = 0;
        for (i; i < m_Length; i++)
            if (m_Buffer[i] == c) return (int)i;
        return -1;
    }

    String& concat(const String& string)
    {
        int stringSize = string.size();
        unsigned int newLength = m_Length + stringSize;
        int len = newLength + 1;
        char* tempStr = new char[len];

        for (unsigned int i = 0; i < newLength; i++)
        {
            tempStr[i] = m_Buffer[i];
        }

        for (int i = 0; i < stringSize; i++)
        {
            tempStr[newLength + i] = string[i];
        }

        delete[] m_Buffer;
        m_Buffer = new char[len];

        for (unsigned int i = 0; i < newLength; i++)
        {
            m_Buffer[i] = tempStr[i];
        }

        m_Length = newLength;
        m_Buffer[newLength] = '\0';

        delete[] tempStr;
        tempStr = nullptr;

        return *this;
    }

    String& concat(const char* string)
    {
        String other(string);
        return this->concat(other);
    }

    String& toUpper()
    {
        char diff = ('a' - 'A');
        unsigned int i = 0;
        for (i; i < m_Length; i++)
        {
            if ('a' <= m_Buffer[i] && m_Buffer[i] <= 'z')
            {
                m_Buffer[i] -= diff;
            }
        }
        return *this;
    }

    String& toLower()
    {
        for (unsigned int i = 0; i < m_Length; i++)
        {
            if ('A' <= m_Buffer[i] && m_Buffer[i] <= 'Z')
            {
                m_Buffer[i] += ('a' - 'A');
            }
        }
        return *this;
    }

    int find(String& string)
    {
        int strSize = string.size();

        if (strSize == 0) return -1;

        int len = strSize - 1;
        int posSearch = 0;
        unsigned int i = 0;
        for (i; i < m_Length; ++i)
        {
            if (m_Buffer[i] == string.c_str()[posSearch])
            {
                ++posSearch;
                if (posSearch == strSize) return i - len;
            }
            else
            {
                i -= posSearch;
                posSearch = 0;
            }
        }

        return -1;
    }

    int find(const char* cString)
    {
        String string(cString);
        return this->find(string);
    }

    String& replace(String& target, String& replacement)
    {
        int startAt = this->find(target);

        if (startAt == -1)
        {
            return *this;
        }

        int replacementSize = replacement.size();
        int targetSize = target.size();
        int newLength = m_Length + replacementSize - targetSize;
        int len = newLength + 1;
        char* tempStr = new char[len];
        int intervalIndex = 0;

        if (replacementSize > 0)
        {
            for (int i = 0; i < newLength; i++)
            {
                if (i >= startAt && intervalIndex < replacementSize)
                {
                    tempStr[i] = replacement[intervalIndex];
                    intervalIndex++;
                }
                else
                {
                    tempStr[i] = m_Buffer[intervalIndex > 0 ? (i + targetSize - intervalIndex) : i];
                }
            }
        }
        else
        {
            for (int i = 0; i < newLength; i++)
            {
                if (i >= startAt && intervalIndex < targetSize)
                {
                    tempStr[i] = m_Buffer[i + targetSize];
                    intervalIndex++;
                }
                else
                {
                    tempStr[i] = m_Buffer[intervalIndex > 0 ? (i + targetSize) : i];
                }
            }
        }

        delete[] m_Buffer;
        m_Length = newLength;
        m_Buffer = new char[len];

        for (unsigned int i = 0; i < m_Length; i++)
        {
            m_Buffer[i] = tempStr[i];
        }
        m_Buffer[newLength] = '\0';

        delete[] tempStr;
        tempStr = nullptr;

        return *this;
    }

    String& replace(String& target, const char* replacement)
    {
        String strReplacement(replacement);
        return this->replace(target, strReplacement);
    }

    String& replace(const char* target, String& replacement)
    {
        String strTarget(target);
        return this->replace(strTarget, replacement);
    }

    String& replace(const char* target, const char* replacement)
    {
        String strTarget(target);
        String strReplacement(replacement);
        return this->replace(strTarget, strReplacement);
    }

    String& erase(int from)
    {
        if (from < 0 || from > static_cast<int>(m_Length))
            return *this;

        memset(&m_Buffer[from], 0, m_Length - from);
        m_Length -= from;
        return *this;
    }

    String& erase_front(int to)
    {
        if (to < 0 || to > static_cast<int>(m_Length))
            return *this;

        memmove(&m_Buffer[0], &m_Buffer[to + 1], m_Length - to);
        m_Length -= to;
        return *this;
    }

    std::string toString()
    {
        return std::string(m_Buffer);
    }

    String& operator=(const String& string)
    {
        if (this == &string) return *this;

        int len = string.size();
        delete[] m_Buffer;
        m_Buffer = new char[len + 1];

        int i = 0;
        for (i; i < len; i++) m_Buffer[i] = string[i];

        m_Length = len;
        m_Buffer[len] = '\0';

        return *this;
    }

    char operator[](unsigned int index) const
    {
        if (index >= m_Length) throw 1;
        return m_Buffer[index];
    }

    char& operator[](unsigned int index)
    {
        if (index >= m_Length) throw 1;
        return m_Buffer[index];
    }

    friend bool operator==(String& left, String& right)
    {
        return left.equals(right);
    }

    friend bool operator==(String& left, const char* right)
    {
        return left.equals(right);
    }

    friend bool operator==(const char* left, String& right)
    {
        return right.equals(left);
    }

    friend bool operator!=(String& left, String& right)
    {
        return !left.equals(right);
    }

    friend bool operator!=(String& left, const char* right)
    {
        return !left.equals(right);
    }

    friend bool operator!=(const char* left, String& right)
    {
        return !right.equals(left);
    }

    friend bool operator<(String& left, String& right)
    {
        return strcmp(left.c_str(), right.c_str()) < 0;
    }

    friend bool operator<(String& left, const char* right)
    {
        return strcmp(left.c_str(), right) < 0;
    }

    friend bool operator<(const char* left, String& right)
    {
        return strcmp(left, right.c_str()) < 0;
    }

    friend String operator+(const String& left, const String& right)
    {
        return String(left).concat(right);
    }

    friend String operator+(const String& left, const char* right)
    {
        return String(left).concat(right);
    }

    friend String operator+(const char* left, const String& right)
    {
        return String(left).concat(right);
    }

    String& operator+=(const String& string)
    {
        return this->concat(string);
    }
};

#endif /* STRING_H */
