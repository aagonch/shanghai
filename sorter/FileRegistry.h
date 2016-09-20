#pragma once

#include <vector>
#include <string>

// stores names of initial and tmp files
class FileRegistry
{
    size_t m_counter = 0;
    std::string m_initialFile;
    std::vector<std::string> m_files;
public:
    FileRegistry(const std::string& initialFile) : m_initialFile(initialFile) {}

    const std::string& GetInitialFile() const { return m_initialFile; }

    size_t Count() const { return m_files.size(); }

    std::string GetNext(const std::string& label = std::string())
    {
        std::string fname = m_initialFile + "." + label + (label.empty() ? "" : ".") + std::to_string(++m_counter);
        m_files.push_back(fname);
        return fname;
    }

    std::vector<std::string> PopFront(size_t count)
    {
        std::vector<std::string> result;
        if (m_files.size() <= count)
        {
            m_files.swap(result);
            return result;
        }
        result.assign(m_files.begin(), m_files.begin() + count);
        m_files.erase(m_files.begin(), m_files.begin() + count);
        return result;
    }
};
