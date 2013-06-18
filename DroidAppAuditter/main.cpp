#include <cstdio>
#include <cstdlib>
#include <windows.h>
#include <vector>

using namespace std;

struct File_Info
{
    char mode[11];
    char name[100];
};

vector<File_Info> _file_infoes;
vector<char *> _data_lines;
vector<File_Info> _finded_file_infos;

void check_the_data(void)
{
    int size_ = _file_infoes.size();
    for (int i = 0; i != size_; ++i)
    {
        if (strncmp(&_file_infoes[i].mode[7], "---", 3))
        {
            File_Info file_info_;
            memset(&file_info_, 0, sizeof(File_Info));
            strncpy(file_info_.mode, _file_infoes[i].mode, strlen(_file_infoes[i].mode));
            strncpy(file_info_.name, _file_infoes[i].name, strlen(_file_infoes[i].name));
            _finded_file_infos.push_back(file_info_);
        }
    }
    size_ = _finded_file_infos.size();
    for (int i = 0; i != size_; ++i)
    {
        printf("%s %s\n", _finded_file_infos[i].mode, _finded_file_infos[i].name);
    }
}


void handle_the_data(char *_data, const int _size)
{
    int start_ = 0;
    int end_ = 0;
    for (int i = 0; i < _size - 2; ++i)
    {
        if (_data[i] == 0x0d && _data[i + 1] == 0x0d && _data[i + 2] == 0x0a)
        {
            if (_data[start_] == '-')
            {
                end_ = i;
                int line_size_ = end_ - start_;
                char *lines = (char *)malloc(sizeof(char) * line_size_ + 1);
                memset(lines, 0, line_size_ + 1);
                memcpy(lines, &_data[start_], line_size_ + 1);
                lines[line_size_] = '\0';
                _data_lines.push_back(lines);
                i += 3;
                start_ = i;
            }
            else
            {
                i += 3;
                start_ = i;
            }
        }
    }

    int _line_num = _data_lines.size();
    for (int i = 0; i != _line_num; ++i)
    {
        File_Info file_info_;
        memset(&file_info_, 0, sizeof(File_Info));
        strncpy(file_info_.mode, _data_lines[i], 10);

        char _temp[100] = {'\0'};
        memset(_temp, 0, 100);
        int _lenght = strlen(_data_lines[i]);
        strncpy(_temp, _data_lines[i], _lenght);
        int _index = 0;
        for (int j = 0; j < _lenght; j++)
        {
            if (_temp[j] == 0x20)
            {
                int count = 0;
                do 
                {
                    ++count;
                } while (_temp[j + count] == 0x20);
                j += count;
                if (++_index == 6)
                {
                    strncpy(file_info_.name, &_temp[j], _lenght - j);
                    break;
                }
            }
        }
        _file_infoes.push_back(file_info_);
    }
    _data_lines.clear();
}

void read_the_file_(const char *_file_name)
{
    HANDLE hFile = CreateFileA(_file_name, GENERIC_READ, FILE_SHARE_READ,
                                NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        DWORD _file_high_size = 0;
        DWORD _file_size = GetFileSize(hFile, &_file_high_size);
        DWORD *_data_addr = (DWORD *)0x20000000;
        DWORD *_data = (DWORD *)VirtualAlloc(_data_addr, _file_size,
                                                MEM_COMMIT | MEM_RESERVE,
                                                PAGE_READWRITE);
        if (_data != NULL)
        {
            DWORD _file_read_size = 0;
            while (1)
            {
                if (!ReadFile(hFile, _data, _file_size, &_file_read_size, NULL))
                {
                    int error = GetLastError();
                    break;
                }

                handle_the_data((char *)_data, _file_read_size);
                check_the_data();

                if (_file_read_size == _file_size)
                {
                    break;
                }
                else if (_file_read_size < _file_size)
                {
                    _file_size -= _file_read_size;
                }
            }
        }
        VirtualFree(_data, NULL, MEM_RELEASE);
        CloseHandle(hFile);
    }
}

int main(int argc, char **argv)
{
    int _in = 0;

    while (1)
    {
        printf("input(0-1):\n");
        scanf("%d", &_in);
        switch (_in)
        {
        case 0:
            return 0;
        default:
            break;
        case 1:
            char _app_name[50] = { '\0' };
            char _command[100] = { '\0' };
            printf("input the app_name:\n");
            scanf("%s", _app_name);
            sprintf(_command, 
                    "adb shell ls /data/data/%s -l -R > ls.txt", _app_name);
            system(_command);
            read_the_file_("ls.txt");
            break;
        }
    }
    return 0;
}