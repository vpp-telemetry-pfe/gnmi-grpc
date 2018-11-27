
/*  vim:set softtabstop=2 shiftwidth=2 tabstop=2 expandtab: */

#include <grpcpp/security/server_credentials.h>

using grpc::ServerCredentials;

const char InsecureCredentialsType[] = "INSECURE_CREDENTIALS";
// For real credentials, like tls/ssl, this name should match the AuthContext
// // property "transport_security_type".
const char TlsCredentialsType[] = "ssl";

std::shared_ptr<ServerCredentials> GetServerCredentials(
    const grpc::string& type);
