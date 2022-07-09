#!/bin/bash

printf "Making new directory public\n"
mkdir -p ./keycloak/keycloak_data

printf "Start docker compose\n"
docker-compose up
