#pragma once 
#include <cstdio>
#include <streambuf>
#include <ostream>
#include <fstream>
#include <iostream>
#include <string>

class TeeStreambuf : public std::streambuf 
{
private:
    std::streambuf* sb1_;
    std::streambuf* sb2_;

protected:
    int overflow(int c) override 
    {
        if (c == EOF) return EOF;
        int r1 = sb1_->sputc(char(c));
        int r2 = sb2_->sputc(char(c));
        return (r1 == EOF || r2 == EOF) ? EOF : c;
    }

    int sync() override 
    {
        int r1 = sb1_->pubsync();
        int r2 = sb2_->pubsync();
        return (r1 == 0 && r2 == 0) ? 0 : -1;
    }

public:
    TeeStreambuf(std::streambuf* sb1, std::streambuf* sb2) : sb1_(sb1), sb2_(sb2) {}
};

class TeeStream : public std::ostream {
private:
    std::ofstream file_;
    TeeStreambuf tbuf_;
public:
    explicit TeeStream(const std::string& outpath)
        : std::ostream(nullptr)
        , file_(outpath)
        , tbuf_(std::cout.rdbuf(),file_.is_open() ? file_.rdbuf() : nullptr)
    {
        this->init(&tbuf_);
    }
};
