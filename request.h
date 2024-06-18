#ifndef __REQUEST_H
#define __REQUEST_H

#include <cstring>
#include <map>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

enum CHECK_STATE { CHECKING_REQLINE, CHECKING_HEADER, CHECKING_BODY };
enum LINE_STATUS { LINE_OK, LINE_BAD, LINE_OPEN };
enum HTTP_CODE { NO_REQ, GET_REQ, BAD_REQ };

const int BUFLEN = 4096;

class Request {
public:
    Request() {}
    Request(int confd)
        : fd_(confd) {}

    HTTP_CODE Parse() {
        char        buf[BUFLEN];
        CHECK_STATE state = CHECK_STATE::CHECKING_REQLINE;
        HTTP_CODE   ret;

        while (1) {
            // TODO: 数据很多, buf可能不够,需要度多次
            int rn = recv(fd_, buf, BUFLEN, 0);
            if (rn < 0) {
                perror("Reading failed.\n");
                break;
            } else if (rn == 0) {
                perror("Remote client has closed connection.\n");
                break;
            }

            LINE_STATUS linestauts;
            char       *bufp        = buf;
            int         read_idx    = 0;
            int         checked_idx = 0;

            while ((linestauts = ParseLine(bufp, read_idx, rn)) == LINE_STATUS::LINE_OK) {

                switch (state) {
                case CHECKING_REQLINE:
                    ret = ParseRequestLine(bufp);
                    if (ret == GET_REQ)
                        state = CHECK_STATE::CHECKING_HEADER;
                    else if (ret == NO_REQ)
                        break;
                    else
                        return ret;
                    break;

                case CHECKING_HEADER:
                    ret = ParseRequestHeader(bufp);
                    if (ret == NO_REQ)
                        break;
                    else
                        return ret;
                    break;

                default: break;
                }

                bufp += read_idx;
            }

            if (linestauts == LINE_STATUS::LINE_BAD)
                return BAD_REQ;
            else if (linestauts == LINE_STATUS::LINE_OPEN)
                continue;
        }
    }

    void handle() {
        HTTP_CODE ret = Parse();
        if (ret == GET_REQ) {
            char header[BUFLEN];
            char resp[BUFLEN] = "Hello from cpp web server.";

            sprintf(header, "HTTP/1.1 %d %s\r\n", 200, "OK");
            sprintf(header, "%sConnection: keep-alive\r\n", header);
            sprintf(header, "%sKeep-Alive: timeout=%d\r\n", header, 500);
            sprintf(header, "%sContent-type: text/plain\r\n", header);
            sprintf(header, "%sContent-length: %ld\r\n\r\n", header, strlen(resp));

            send(fd_, header, strlen(header), 0);
            send(fd_, resp, strlen(resp), 0);
        }
    }

    void SetFd(int fd) { fd_ = fd; }
    int  Fd() { return fd_; }

private:
    LINE_STATUS ParseLine(char *buf, int &tail, int nread) {
        int c;
        for (int i = 0; i < nread; i++) {
            c = buf[i];
            if (c == '\r') {
                if (i + 1 == nread)
                    return LINE_STATUS::LINE_OPEN;
                else if (buf[i + 1] == '\n') {
                    buf[i]     = '\0';
                    buf[i + 1] = '\0';
                    tail       = i + 2;
                    return LINE_STATUS::LINE_OK;
                }
                return LINE_STATUS::LINE_BAD;
            } else if (c == '\n') {
                if (i > 0 && buf[i - 1] == '\r') {

                    buf[i - 1] = '\0';
                    buf[i]     = '\0';
                    tail       = i + 1;
                    return LINE_STATUS::LINE_OK;
                }
                return LINE_STATUS::LINE_BAD;
            }
        }
        return LINE_STATUS::LINE_OPEN;
    }

    HTTP_CODE ParseRequestLine(char *buf) {
        char *p = strpbrk(buf, " \t");
        if (!p) return HTTP_CODE::BAD_REQ;
        *p++ = '\0';

        // method
        char *method = buf;
        if (strcasecmp(method, "GET") == 0)
            printf("Method is GET\n");
        else
            return BAD_REQ;

        // url
        char *url = p;
        p         = strpbrk(p, " \t");
        if (!p) return HTTP_CODE::BAD_REQ;
        *p++ = '\0';
        printf("Url is %s\n", url);

        // version
        char *version = p;
        if (strcasecmp(version, "HTTP/1.1") == 0)
            printf("Version is HTTP/1.1\n");
        else
            return BAD_REQ;

        return GET_REQ;
    }

    HTTP_CODE ParseRequestHeader(char *buf) {
        // \r\n
        if (buf[0] == '\0') return GET_REQ;

        char *p = strpbrk(buf, " \t");
        if (!p) return HTTP_CODE::BAD_REQ;
        *p++ = '\0';

        char *key   = buf;
        char *value = p;
        printf("%s %s\n", key, value);
        return NO_REQ;
    }


    int         fd_;
    std::string content_;   // body

    std::string                        method_;
    std::string                        url_;
    std::map<std::string, std::string> req_headers_;
};


#endif