import logging
from tokenize import String
import keycloak as _keycloak
import os as _os

log = logging.getLogger(__name__)

# https://dev.to/tomas223/logging-tracing-in-python-fastapi-with-opencensus-a-azure-2jcm
def login_user(username: String, password: String) -> String:
    try:
        keycloak_openid = _keycloak.KeycloakOpenID(server_url= _os.environ['KEYCLOAK_SERVER_URL'],
                        client_id=_os.environ['KEYCLOAK_CLIENT_ID'],
                        realm_name=_os.environ['KEYCLOAK_REALM_NAME'],
                        client_secret_key=_os.environ['KEYCLOAK_CLIENT_SECRET_KEY'])

    except:
        log.error("Keycloak server unreachable")
        return None
    
    try:
        # Get Token
        token = keycloak_openid.token(username=username, password=password)

        return token['access_token']
    
    except:
        log.warning("Unauthorized")
        return None