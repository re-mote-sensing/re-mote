curl -L https://gitlab.cas.mcmaster.ca/re-mote/pi-server/-/jobs/artifacts/master/download?job=build > pi-server-frontend.zip
unzip pi-server-frontend.zip
rm -rf static/
mkdir static/
mv macwater-webserver/frontend/dist/* static/
rm -rf macwater-webserver/
rm pi-server-frontend.zip