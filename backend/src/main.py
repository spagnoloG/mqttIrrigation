import fastapi as _fastapi
import init.keycloak as _keycloak
from dotenv import load_dotenv
import os as _os


load_dotenv()
print(_keycloak.login_user("user", "user"))

app = _fastapi.FastAPI()
