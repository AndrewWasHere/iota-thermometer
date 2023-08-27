#ifndef IOTA_SECRETS_H
#define IOTA_SECRETS_H

struct Secrets {
    char const * ssid;
    char const * password;
    char const * iota_id;
    char const * iota_ip;
    int  const   iota_port;
};

extern Secrets secrets;
#endif
