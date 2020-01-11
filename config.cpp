#include "config.h"
#include <fstream>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

inline std::vector<std::string> split(const std::string& s, char seperator)
{
    std::vector<std::string> output;
    std::string::size_type prev_pos = 0, pos = 0;
    while((pos = s.find(seperator, pos)) != std::string::npos)
    {
        std::string substring( s.substr(prev_pos, pos-prev_pos) );
        output.push_back(substring);
        prev_pos = ++pos;
    }
    output.push_back(s.substr(prev_pos, pos-prev_pos)); // Last word
    return output;
}

inline std::string trim(const std::string& src, const std::string &remove = " \n\r\t")
{
    std::string str(src);
    str.erase(0, str.find_first_not_of(remove));       //prefixing spaces
    str.erase(str.find_last_not_of(remove)+1);         //surfixing spaces
    return str;
}

inline bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

std::vector<std::string> listdir(std::string dir)
{
    std::vector<std::string> files;
    DIR *dp;
    struct dirent *dirp;
    dp  = opendir(dir.c_str());

    if(dp)
    {
        while ((dirp = readdir(dp)) != nullptr)
        {
            files.push_back(std::string(dirp->d_name));
        }
        closedir(dp);
    }

    return files;
}

Config::Config(std::string id)
    : m_id(id)
{
    std::string home = std::getenv("HOME");

    // Parse: /etc/<id>.conf and /etc/<id>.conf.d/*.conf
    _parsePath("/etc/");

    // Parse: ~/.<id>.conf and ~/.<id>.conf.d/*.conf
    _parsePath(home + "/.");

    // Parse: ~/.config/<id>.conf and ~/.config/<id>.conf.d/*.conf
    _parsePath(home + "/.config/");
}

const std::string &Config::operator[](const std::string &id) const
{
    if(m_map.count(id))
    {
        return m_map.at(id);
    }
    return m_empty;
}

const std::vector<std::string>& Config::keys() const
{
    if(m_keys.size() != m_map.size())
    {
        for(std::map<std::string,std::string>::const_iterator it = m_map.begin(); it != m_map.end(); ++it)
        {
            m_keys.push_back(it->first);
        }
    }
    return m_keys;
}

const std::vector<std::string> &Config::arrays() const
{
    if(m_keys_arrays.size() != m_map_arrays.size())
    {
        for(std::map<std::string,std::vector<std::string>>::const_iterator it = m_map_arrays.begin(); it != m_map_arrays.end(); ++it)
        {
            m_keys_arrays.push_back(it->first);
        }
    }
    return m_keys_arrays;
}

const std::vector<std::string> &Config::array(const std::string &id)
{
    if(m_map_arrays.count(id))
    {
        return m_map_arrays.at(id);
    }
    return m_empty_array;
}

int Config::_parsePath(std::string path)
{
    struct stat st;
    int count = 0;
    path += m_id + ".conf";
    count += _parseFile(path);

    path += ".d";
    if(stat(path.c_str(),&st) == 0 && ((st.st_mode & S_IFDIR) != 0))
    {
        for(auto& p: listdir(path))
        {
            if(ends_with(path + "/" + p, ".conf"))
            {
                count += _parseFile(path + "/" + p);
            }
        }
    }
    return count;
}

int Config::_parseFile(std::string file)
{
    std::string line;
    std::string url;
    std::string topic; /* [General] is the root topic */
    int count = 0;
    bool add = false;
    bool addarray = false;

    if(access(file.c_str(), R_OK) != 0)
    {
        return 0;
    }

    std::ifstream infile(file);
    while (std::getline(infile, line))
    {
        line = trim(line);
        if(line[0] == '#')
            continue;

        if(line[0] == '[') /* Topic line */
        {
            topic = trim(line, "[]");
            if(topic == "General")
                topic.clear();
            continue;
        }

        std::vector<std::string> lineconf = split(line, '=');
        if(lineconf.size() != 2)
            continue;

        std::string id = (topic.size())?(topic + "/" + lineconf[0]):(lineconf[0]);
        if(lineconf[0][lineconf[0].size() - 1] == ']')
        {
            if(m_map_arrays.count(trim(id, "[]")) == 0)
            {
                m_map_arrays[trim(id, "[]")] = {};
            }
            m_map_arrays[trim(id, "[]")].push_back(lineconf[1]);
            addarray = true;
            continue;
        }

        ++count;
        add = true;
        m_map[id] = lineconf[1];
    }

    if(add && m_keys.size())
        m_keys.clear();
    if(addarray && m_keys_arrays.size())
        m_keys_arrays.clear();
    return count;
}

