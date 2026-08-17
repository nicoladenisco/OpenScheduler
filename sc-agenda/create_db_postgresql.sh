#!/bin/bash

USER="agenda"
DBNAME="agenda2"

HOST="-h localhost -p 5432"

# caso speciale macchina virtuale di sviluppo
if [ "$HOSTNAME" == "devdm-VirtualBox" ]; then
  HOST="-h 192.168.56.1 -p 5434"
fi

PSQL="psql $HOST"
DROPDB="dropdb $HOST"
CREATEDB="createdb $HOST"

LOGFILE="creazione-agenda.log"
SQL_SCRIPT=docs/sample-postgresql-data
GEN_SCRIPT=target/generated-sql/torque/postgresql

if [ ! -d $GEN_SCRIPT ]; then
  GEN_SCRIPT=generated-sql/torque/postgresql
fi

echo "========================="
echo "Creazione database FLOWER"
echo "========================="
echo "HOST=$HOST"
echo "DATABASE=$DBNAME"
echo "USER=$USER"
echo ""
echo ""
echo "D - distrugge db e ricrea"
echo "C - crea db e struttura"
echo "P - crea struttura e popola db"
echo -n "Scegli [d/c/p]:"

read RESP

case $RESP in
    d|D)
	echo "Distruggo il vecchio database $DBNAME."
	$DROPDB -U $USER $DBNAME
        if [ $? -ne 0 ]; then
          echo "Non posso distruggere il vecchio db."
          exit $?
        fi

	$CREATEDB -E utf8 -U $USER $DBNAME
	;;
    c|C)
        echo "Creo database $DBNAME."
	$CREATEDB -E utf8 -U $USER $DBNAME
	;;
    p|P)
	echo "Generazione struttura su database esistente."
	;;
    *)
	echo "Opzione non consentita."
	exit -1
esac

rm -f $LOGFILE
#set -x
echo "Costruzione struttura in corso ..."

echo "  eseguo file ${SQL_SCRIPT}/agenda-pre.sql"
$PSQL -f ${SQL_SCRIPT}/agenda-pre.sql -U $USER $DBNAME >> $LOGFILE 2>&1

for f in torque-security-schema.sql id-table-schema.sql application-schema.sql torque-security-viste.sql id-table-viste.sql application-viste.sql
do
  echo "  eseguo file ${GEN_SCRIPT}/$f"
  echo "  eseguo file ${GEN_SCRIPT}/$f" >> $LOGFILE 2>&1
  cat ${GEN_SCRIPT}/$f \
   | sed 's/CONSTRAINT ..\./CONSTRAINT /' \
   | sed 's/CONSTRAINT ...\./CONSTRAINT /' \
   | sed 's/CONSTRAINT ....\./CONSTRAINT /' \
   | $PSQL -U $USER $DBNAME >> $LOGFILE 2>&1
done

for f in agenda-extra.sql agenda-view.sql
do
  echo "  eseguo file ${SQL_SCRIPT}/$f"
  echo "  eseguo file ${SQL_SCRIPT}/$f" >> $LOGFILE 2>&1
  $PSQL -f ${SQL_SCRIPT}/$f -U $USER $DBNAME >> $LOGFILE 2>&1
done
echo "fatto!"

echo "Popolamento con dati essenziali ..."
while read -r f
do
  # salta righe vuote e commenti
  if [ -z "$f" ] || [ "${f:0:1}" = "#" ]; then
    continue
  fi

  echo "  eseguo file ${SQL_SCRIPT}/$f"
  echo "  eseguo file ${SQL_SCRIPT}/$f" >> $LOGFILE 2>&1

  case "$f" in
    *.sql)
      $PSQL -f "${SQL_SCRIPT}/$f" -U $USER $DBNAME >> $LOGFILE 2>&1
      ;;
    *.csv)
      # la tabella ha lo stesso nome del file csv senza estensione
      TABLE=$(basename "$f" .csv)
      python3 "${SQL_SCRIPT}/csv2insert.py" "${SQL_SCRIPT}/$f" --table "$TABLE" --batch 100 \
        | $PSQL -U $USER $DBNAME >> $LOGFILE 2>&1
      ;;
    *)
      echo "  estensione non gestita: $f" | tee -a $LOGFILE
      ;;
  esac
done < ${SQL_SCRIPT}/populate-list.txt
echo "fatto!"
