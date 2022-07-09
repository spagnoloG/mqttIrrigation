import fastapi as _fastapi
import services.auth as _auth 
from dotenv import load_dotenv
import os as _os


load_dotenv()

_auth.login_user("user", "user")

app = _fastapi.FastAPI()
