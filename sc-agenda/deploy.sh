#!/bin/bash
. ../../common-copy.sh

USER="$USER"
HOST="localhost"
APP="agenda"
ORIGIN="target/sc-agenda-0.1-SNAPSHOT"
GENSQL="target/generated-sql"
UPDATESQL="docs/updatedb/sql"

if [ -n "$1" ]; then
    USER="$1"
    shift
fi

if [ -n "$1" ]; then
    HOST="$1"
    shift
fi

if [ -n "$1" ]; then
    APP="$1"
fi

clear
manifesto "SC-AGENDA"

TESTAPP=$(ssh $USER@$HOST "ls -1 /usr/local/tdk/webapps/" | grep $APP)
if [ -z "$TESTAPP" ]; then
  printf "\n${PURPLE}ATTENZIONE: l'applicazione $APP non esiste nel server indicato.\n"
  printf "Vuoi davvero creare una nuova applicazione in $HOST? (S/n):${NC}"
  read risposta

  case $risposta in
    s|S)
            printf "\n${PURPLE}Inizio deploy nuova applicazione ... ${NC}\n"
    ;;
    n|N)
            printf "\n${PURPLE}Bye. ${NC}\n"
            exit -1
    ;;
    *)
            printf "\n${PURPLE}Opzione non consentita. ${NC}\n"
            exit -1
  esac
else
  printf "\n${PURPLE}Deploy applicazione in corso ... ${NC}\n"
fi

DDIR="/usr/local/tdk/webapps/$APP"
COPYCMD="rsync -azv --chmod=a+r,Da+x"
DEST="$USER@$HOST:$DDIR"

rm -f ${ORIGIN}/logs/*.log

# cancella conflitti fra versioni diverse di librerie
rm -f ${ORIGIN}/WEB-INF/lib/servlet-api-2.4.jar

#set -x

# sincronizzazione della directory applicazione
copiaSilente ${ORIGIN}/* $DEST

# templates,classes,lib,components devono essere sincronizzate con precisione
# ovvero cancellando eventualmente i file remoti non piu' usati
copiaSilente --delete ${ORIGIN}/templates $DEST/
copiaSilente --delete ${ORIGIN}/WEB-INF/classes $DEST/WEB-INF/
copiaSilente --delete ${ORIGIN}/WEB-INF/lib $DEST/WEB-INF/
# copiaSilente --delete ${ORIGIN}/WEB-INF/conf/components $DEST/WEB-INF/conf/

# sincronizzazione della directory script SQL
ssh -T $USER@$HOST "mkdir -p ${DDIR}/WEB-INF/db"
copiaSilente --delete ${GENSQL} $DEST/WEB-INF/db
copiaSilente --delete --exclude updatedb docs $DEST/WEB-INF/db
copiaSilente --delete create_db_* $DEST/WEB-INF/db

# sincronizzazione della directory aggiornamenti SQL
#ssh -T $USER@$HOST "mkdir -p ${DDIR}/WEB-INF/sql"
copiaSilente --delete ${UPDATESQL} $DEST/WEB-INF/conf

ssh -T $USER@$HOST << EOF
#set -x

# cancella servlet api
rm -f $DDIR/WEB-INF/lib/servletapi-2.4.jar

# rende di nuovo eseguibili gli script di servizio
# chmod a+x ${DDIR}/WEB-INF/conf/*.sh

# attiva ripristino corretto configurazione locale
chmod a+x ${DDIR}/WEB-INF/file-locali/ripristina.sh
${DDIR}/WEB-INF/file-locali/ripristina.sh

EOF

epilogo
