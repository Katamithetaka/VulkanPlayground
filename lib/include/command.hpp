#ifndef LIB_COMMAND_HPP
#define LIB_COMMAND_HPP

#include <vector>
#include <string>
#include <array>

#ifdef _WIN32

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

bool runProgram(const std::string &command, const std::vector<std::string> &args = std::vector<std::string>{}, std::string *stdout_pipe = nullptr, std::string *stderr_pipe = nullptr)
{
    HANDLE g_hChildStd_ERR_Rd = NULL;
    HANDLE g_hChildStd_ERR_Wr = NULL;
    HANDLE g_hChildStd_OUT_Rd = NULL;
    HANDLE g_hChildStd_OUT_Wr = NULL;

    SECURITY_ATTRIBUTES saAttr;

    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);

    saAttr.bInheritHandle = TRUE;

    saAttr.lpSecurityDescriptor = NULL;

    // Create a pipe for the child process's STDOUT.

    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
        return false;
    // Ensure the read handle to the pipe for STDERR is not inherited.

    if (!SetHandleInformation(g_hChildStd_ERR_Rd, HANDLE_FLAG_INHERIT, 0))
        return false;
    if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
        return false;
    // Ensure the read handle to the pipe for STDERR is not inherited.

    if (!SetHandleInformation(g_hChildStd_ERR_Rd, HANDLE_FLAG_INHERIT, 0))
        return false;
    std::string finalCommand = "";
    finalCommand += command;

    for (auto &arg : args)
    {
        finalCommand += " " + arg;
    }

    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE;

    // Set up members of the PROCESS_INFORMATION structure.

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    // Set up members of the STARTUPINFO structure.
    // This structure specifies the STDIN and STDOUT handles for redirection.

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = g_hChildStd_ERR_Wr;
    siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    bSuccess = CreateProcessA(
        NULL,
        finalCommand.data(),
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &siStartInfo, // STARTUPINFO pointer
        &piProcInfo); // receives PROCESS_INFORMATION

    if (!bSuccess)
        return false;
    else
    {
        // Close handles to the child process and its primary thread.
        // Some applications might keep these handles to monitor the status
        // of the child process, for example.

        CloseHandle(piProcInfo.hProcess);
        CloseHandle(piProcInfo.hThread);

        // Close handles to the stdin and stdout pipes no longer needed by the child process.
        // If they are not explicitly closed, there is no way to recognize that the child process has ended.

        CloseHandle(g_hChildStd_OUT_Wr);
        CloseHandle(g_hChildStd_ERR_Wr);
    }

    // read stdout and stderr

    constexpr size_t BUFSIZE = 4096;
    DWORD dwRead, dwWritten;
    CHAR chBuf[BUFSIZE];
    bSuccess = FALSE;

    if (stdout_pipe)
    {

        for (;;)
        {
            bSuccess = ReadFile(g_hChildStd_OUT_Rd, chBuf, BUFSIZE, &dwRead, NULL);
            if (!bSuccess || dwRead == 0)
                break;

            *stdout_pipe += std::string(chBuf);

            if (!bSuccess)
                break;
        }
    }

    if (stderr_pipe)
    {

        for (;;)
        {
            bSuccess = ReadFile(g_hChildStd_ERR_Rd, chBuf, BUFSIZE, &dwRead, NULL);
            if (!bSuccess || dwRead == 0)
                break;

            *stderr_pipe += std::string(chBuf);

            if (!bSuccess)
                break;
        }
    }

    return true;
}
#else

#include <unistd.h>
#include <filesystem>
#include <iterator>
#include <fstream>

std::string generate_random_file()
{
    constexpr const char letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
    constexpr int size = std::size(letters);
    constexpr int number_characters = 15;

    while (true)
    {

        std::string path = "/tmp/";
        for (int i = 0; i < number_characters; ++i)
        {
            path += letters[rand() % size];
        }

        if (std::filesystem::exists(path))
        {
            continue;
        }

        return path;
    }
}

bool runProgram(const std::string &command, const std::vector<std::string> &args = std::vector<std::string>{}, std::string *stdout_pipe = nullptr, std::string *stderr_pipe = nullptr)
{
    std::array<char, 128> buffer;
    std::string result;

    auto cmd = command;

    for (auto &arg : args)
    {
        cmd += " " + arg;
    }

    std::string error_file_name = generate_random_file();
    std::string out_file_name = generate_random_file();

    cmd += ">" + out_file_name + " 2>" + error_file_name;

    auto pipe = popen(cmd.c_str(), "r"); // get rid of shared_ptr

    if (!pipe)
        return false;

    auto rc = pclose(pipe);

    if (stdout_pipe)
    {
        std::ifstream stdout_file{out_file_name};
        stdout_file.seekg(0, std::ios::end);
        size_t size = stdout_file.tellg();
        std::string buffer(size, ' ');
        stdout_file.seekg(0);
        stdout_file.read(&buffer[0], size);

        *stdout_pipe = std::move(buffer);
    }

    if (stderr_pipe)
    {
        std::ifstream stderr_file{error_file_name};
        stderr_file.seekg(0, std::ios::end);
        size_t size = stderr_file.tellg();
        std::string buffer(size, ' ');
        stderr_file.seekg(0);
        stderr_file.read(&buffer[0], size);

        *stderr_pipe = std::move(buffer);
    }

    std::filesystem::remove(out_file_name);
    std::filesystem::remove(error_file_name);

    if (rc == EXIT_SUCCESS)
    { // == 0
        return true;
    }
    else
    { // EXIT_FAILURE is not used by all programs, maybe needs some adaptation.
        return false;
    }
}

#endif

#endif