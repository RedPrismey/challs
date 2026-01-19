# Instructions

Merci à pwnsard et à l'équipe du 404ctf de me m'avoir aidé à setup tout

## nsjail container

`git clone https://github.com/google/nsjail.git`
`cd nsjail`
`docker build -t nsjail .`

## challenge container

Modifier le nom du chall + le port host dans `docker-compose.yaml`

`cd pwn-docker`
`docker compose up`
