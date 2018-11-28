
/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <grpcpp/security/server_credentials.h>

using grpc::ServerCredentials;
using grpc::SslServerCredentialsOptions;

// PEM path for server private key & server certificate chain.
struct encrypt_t {
  std::string private_key;
  std::string chain_certs;
};

struct auth_t {
  std::string username;
  std::string password;
};

extern struct encrypt_t encrypt;
extern struct auth_t auth;

const char InsecureCredentialsType[] = "INSECURE_CREDENTIALS";
// For real credentials, like tls/ssl, this name should match the AuthContext
// // property "transport_security_type".
const char TlsCredentialsType[] = "ssl";

std::shared_ptr<ServerCredentials> GetServerCredentials(
    const grpc::string& type);
