//
//      _
//  ___/_)
// (, /      ,_   _
//   /   (_(_/ (_(_(_
// CX________________
//                   )
//
// Luna
// A web application and API framework in modern C++
//
// Copyright © 2016–2017 D.E. Goodman-Wilson
//


#include <gtest/gtest.h>
#include <luna/luna.h>
#include <cpr/cpr.h>

const char* key_pem = R"key(
-----BEGIN RSA PRIVATE KEY-----
MIICXAIBAAKBgQC2WAYBOrf1oVsqSTDuV7PCmbFCfFDdUyceVvoRFYlfKVG3oqzk
ihj74Gz9QN9A5p343wMTPR8T6pUF2vlsnhhWjZWjRtl3Q1ByeR68p3pX1x/VukL6
DpN9/hPja9SQ0O0SN3+nnazUqOWx4xNubjE2qtliRBruYJBToweAumgIWwIDAQAB
AoGAMg27qD68HLssUVFCE0sT10llNUD22apnxJZPkrz8tpibC/Olzh5yeUHD6UkT
f7dOifB94ZwXmdn9Clxvt3c2aqUlban29JCVIv4jNn3xSyX7x1SGSYkVfmdZmx8N
kqJzc3Cd9qqThpICMZorrJgiunvjv2lPez/WYBfopo1HHbECQQDlBpq2xvIxrEB9
xpshG7g92PFC/cBRpBiSddj9SNTE51KFfbwWQXATvEeAuCvNDvt4DMGhf8uR1RQ3
Y1u1Jf+NAkEAy9HnTSdJyvNK1oOGHY21qmzeQdsf9kSVwyk0ZrzjzjvpBMdGDanu
pmXXnAb6g4two1Pv/usZ8AkH1WOPze+ZhwJBALt+9ahPJTZfU4jk0FmBjQstEQ8M
GKd91RQKYLOzLWVqD5fzwMjjl7LAgDU0Eb/V1S+KGfdyXSEttVhHAmxCCH0CQCcB
bqCHSkn6sXHIDdyOCBwrZYV45+77RSXqNssKjxYixP9TG4b15GQXzWFgvIR8JPw8
zgsc9wPG1qbxvM2OcvUCQFaH28XQNEMgo/BI2dFJ9vp8lkrO2mlUHTUjdswGbO7K
3qxwR1O70oBS1gcq/xmKI0VXS0ctKuhL8OcRybgaK7w=
-----END RSA PRIVATE KEY-----
)key";
const char* cert_pem = R"key(
-----BEGIN CERTIFICATE-----
MIIDhzCCAvCgAwIBAgIJAKgEdzixR0vmMA0GCSqGSIb3DQEBBQUAMIGKMQswCQYD
VQQGEwJVUzESMBAGA1UECBMJQ29uZnVzaW9uMQ4wDAYDVQQHEwVDaGFvczEQMA4G
A1UEChMHRGlzY29yZDENMAsGA1UECxMETm9wZTESMBAGA1UEAxMJbG9jYWxob3N0
MSIwIAYJKoZIhvcNAQkBFhNleGFtcGxlQGV4YW1wbGUuY29tMB4XDTE2MTEwMTE0
NDgyNFoXDTE2MTIwMTE0NDgyNFowgYoxCzAJBgNVBAYTAlVTMRIwEAYDVQQIEwlD
b25mdXNpb24xDjAMBgNVBAcTBUNoYW9zMRAwDgYDVQQKEwdEaXNjb3JkMQ0wCwYD
VQQLEwROb3BlMRIwEAYDVQQDEwlsb2NhbGhvc3QxIjAgBgkqhkiG9w0BCQEWE2V4
YW1wbGVAZXhhbXBsZS5jb20wgZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBALZY
BgE6t/WhWypJMO5Xs8KZsUJ8UN1TJx5W+hEViV8pUbeirOSKGPvgbP1A30Dmnfjf
AxM9HxPqlQXa+WyeGFaNlaNG2XdDUHJ5HrynelfXH9W6QvoOk33+E+Nr1JDQ7RI3
f6edrNSo5bHjE25uMTaq2WJEGu5gkFOjB4C6aAhbAgMBAAGjgfIwge8wHQYDVR0O
BBYEFMfiri7IVgqvj0h9xbrK6WuZEBtaMIG/BgNVHSMEgbcwgbSAFMfiri7IVgqv
j0h9xbrK6WuZEBtaoYGQpIGNMIGKMQswCQYDVQQGEwJVUzESMBAGA1UECBMJQ29u
ZnVzaW9uMQ4wDAYDVQQHEwVDaGFvczEQMA4GA1UEChMHRGlzY29yZDENMAsGA1UE
CxMETm9wZTESMBAGA1UEAxMJbG9jYWxob3N0MSIwIAYJKoZIhvcNAQkBFhNleGFt
cGxlQGV4YW1wbGUuY29tggkAqAR3OLFHS+YwDAYDVR0TBAUwAwEB/zANBgkqhkiG
9w0BAQUFAAOBgQBVm72s0mcOPEWUIt4IjemgCsuWC8PV8Fkqg4htuAu/K4cRWhM9
x4gFn/plNA1iaaorcn/LLFt4jv6CTB0KbEZsMgC4I3oGWECamg7CyfXKacvg34Ft
T5NU9xUNbQugJdCMSm+5TLeU5UhsvGqxVDAcIr3w9Iwsyti9CzRs6TzUog==
-----END CERTIFICATE-----
)key";

const char* key_password_pem = R"key(
-----BEGIN RSA PRIVATE KEY-----
Proc-Type: 4,ENCRYPTED
DEK-Info: DES-EDE3-CBC,70D217C07B4421FE

A9KDQkoz6w2bVNCdhBHYIpW0+VcMWXGD/bVswQClAuGyAy+Rfolt0mamwz2YjGxE
WMrzLr+btckEmZgDjkNXZlPuzE7KNKtsRpc5kZ8a2h6Nu91jcATpxBDMpko9UV2Z
dMP5wYIZlijiCJDltyvwuEQJmmex4SK0NSs30lI7rrk8BqEJwlKNfcgP82IGyXiu
YkbF7y7Ubb0lhpb4m64f72xRGOCy9LEklS7x3BDc8WAVsoMaabjTLxPCucaI1Y/v
hQXhA34PLLMXU7DFC1j1NBVIqNZNpZ3rTsmpsZvfPE+UBeYCmdoU4tLD1LcJVoB1
ryphW+ByabHfoR2/RjVvPLhJrDnZsP4rhK3mGeCYD1LakNpDl1kqmx+RZg112kpi
NzlmruoX9s5EU8ggL5bajRsxtsV0fVoye5QsRWsBLZRDLbY4vjGL9kWLqRDcUj+W
nLMUrc+T3TEsBHhSVjlagIz42ELbRu4xMDQrRj9wGDbi8Z+39goRUOKITIs3BkqT
PiiIWScMxm8Oax1mIVMTV+JNZymFaD2kspBs5fgVEgIMzZTMnVPPNiWJq90XsmaQ
BBhr37nwr5WVnzn3p+yDl4z8tANJNE4ybb8pGBgVmv/+k9izYIlI/Enpjz7CkjGC
jLIVWu8tQD25h7W1FyskTrlm4qRKPDMH+r/bWiwQ53WXGk+CxIy7xJH38gYorEd3
t5j1OjnRRJKXB7WFcm0ps11B9IKOdG5tMTbdDmdaZ5EmzZzQkC26zm+sQk4f1hjh
oNqx3wpS8hvgbrBpWubDUS7JwFqWSzKvmes9seRXED39KxqkUOyjnw==
-----END RSA PRIVATE KEY-----
)key";
const char* cert_password_pem = R"key(
-----BEGIN CERTIFICATE-----
MIIDVTCCAr6gAwIBAgIJAM6k79vdf48CMA0GCSqGSIb3DQEBBQUAMHsxCzAJBgNV
BAYTAlVTMRIwEAYDVQQIEwlDb25mdXNpb24xEDAOBgNVBAcTB0Rpc2NvcmQxDjAM
BgNVBAoTBUNoYW9zMRIwEAYDVQQDEwlsb2NhbGhvc3QxIjAgBgkqhkiG9w0BCQEW
E2V4YW1wbGVAZXhhbXBsZS5jb20wHhcNMTYxMTA0MTExNTM5WhcNMTYxMjA0MTEx
NTM5WjB7MQswCQYDVQQGEwJVUzESMBAGA1UECBMJQ29uZnVzaW9uMRAwDgYDVQQH
EwdEaXNjb3JkMQ4wDAYDVQQKEwVDaGFvczESMBAGA1UEAxMJbG9jYWxob3N0MSIw
IAYJKoZIhvcNAQkBFhNleGFtcGxlQGV4YW1wbGUuY29tMIGfMA0GCSqGSIb3DQEB
AQUAA4GNADCBiQKBgQDfE1+pFGiwum0xc81SpiTwrx57ntfYqccYMOV93MEUOhrc
d6ftwi0tSSdHdYYUf3C+vP3TF5W6IlVcvgdbuH4NktWlv14tYvP+8NilUdUIwbrZ
8mqNowOvdGBVnNhGJCymokW8iQCrXNoJlDOvwX0Ry5Kl/2SXoRqO6y72tfSATQID
AQABo4HgMIHdMB0GA1UdDgQWBBTMzyCsshhk+qWYGlNHcdwqt/T2SjCBrQYDVR0j
BIGlMIGigBTMzyCsshhk+qWYGlNHcdwqt/T2SqF/pH0wezELMAkGA1UEBhMCVVMx
EjAQBgNVBAgTCUNvbmZ1c2lvbjEQMA4GA1UEBxMHRGlzY29yZDEOMAwGA1UEChMF
Q2hhb3MxEjAQBgNVBAMTCWxvY2FsaG9zdDEiMCAGCSqGSIb3DQEJARYTZXhhbXBs
ZUBleGFtcGxlLmNvbYIJAM6k79vdf48CMAwGA1UdEwQFMAMBAf8wDQYJKoZIhvcN
AQEFBQADgYEAj6Bw/Am7pupoPVidCbGSxPpCRV4b7sfyalhdBByCueG/WuFpPRRF
inEd1GNE/Xh+75LqCFaFjZeY0Pd7RXQ2qYvTe3eG/3cpnTUJoC0aN7K3AU9XqK0L
1vNl4nGHh2z4G0G8pV8H0kzGIKzrybZiScins3odHuBy0pu3BpBmV4A=
-----END CERTIFICATE-----
)key";

TEST(tls, set_up_https)
{
    luna::set_error_logger([&](luna::log_level level, const std::string& message)
                           {
                               std::cout << ">>>>> [" << to_string(level) << "] " << message << std::endl;
                           });


    luna::server server{luna::server::https_mem_key{key_pem}, luna::server::https_mem_cert{cert_pem}};

    auto router = server.create_router("/");
    router->handle_request(luna::request_method::GET,
                          "/test",
                          [](auto req) -> luna::response
                              {
                                  return {"hello"};
                              });

    server.start_async();

    luna::reset_error_logger();

    ASSERT_TRUE(static_cast<bool>(server));

    auto res = cpr::Get(cpr::Url{"https://localhost:8080/test"}, cpr::VerifySsl{false});

    ASSERT_EQ(200, res.status_code);
    ASSERT_EQ("hello", res.text);
}

TEST(tls, set_up_https_with_password)
{
    luna::server server{luna::server::https_mem_key{key_password_pem}, luna::server::https_mem_cert{cert_password_pem}, luna::server::https_key_password{"foobar"}};
    server.start_async();

    ASSERT_TRUE(static_cast<bool>(server));
}

TEST(tls, set_up_https_with_password_fail)
{
    luna::server server{luna::server::https_mem_key{key_password_pem}, luna::server::https_mem_cert{cert_password_pem}, luna::server::https_key_password{"notapassword"}};
    server.start_async();

    ASSERT_FALSE(static_cast<bool>(server));
}

TEST(tls, set_up_https_fail_1)
{
    luna::server server{luna::server::https_mem_key{key_pem}};
    server.start_async();

    ASSERT_FALSE(static_cast<bool>(server));
}

TEST(tls, set_up_https_fail_2)
{
    luna::server server{luna::server::https_mem_cert{cert_pem}};
    server.start_async();

    ASSERT_FALSE(static_cast<bool>(server));
}