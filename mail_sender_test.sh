#!/bin/sh

WORK_DIR=${HOME}/mail_sender_test

which sendmail > /dev/null 2>&1
if [ $? -ne 0 ]; then
	echo "You need sendmail to run this tool."
	exit 1
fi


if [ ! -e ${WORK_DIR} -o ! -d ${WORK_DIR} ]; then
	echo "WORK_DIR:[${WORK_DIR}] is not found."
	exit 1
fi


MAIL_TEXT=${WORK_DIR}/.tmpmail.txt
DATE_STR=`date +"%Y.%m.%d %H:%M:%S"`

MAIL_TO="<hoge@hoge.com>"
MAIL_CC="<fuga@fuga.com>"
MAIL_FROM="<foo@bar.com>"
#MAIL_SUB="テスト送信 ${DATE_STR}"
MAIL_SUB="テスト送信"
MAIL_BODY="\
メール送信テスト。\r\n\
返信不可。\r\n\
\r\n\
\r\n\
--------\r\n\
`uname -a`\r\n\
${DATE_STR} sent.\r\n\
"


if [ -f "${MAIL_TEXT}" -a -r "${MAIL_TEXT}" ]; then
	echo "${MAIL_TEXT} found."
else
	echo "${MAIL_TEXT} create."
	touch ${MAIL_TEXT}
fi

# header
echo "Content-Type: text/plain; charset=utf-8"           > ${MAIL_TEXT}
echo "Content-Transfer-Encoding: base64"                 >> ${MAIL_TEXT}
echo "To: ${MAIL_TO}"                                    >> ${MAIL_TEXT}
echo "Cc: ${MAIL_CC}"                                    >> ${MAIL_TEXT}
echo "From: ${MAIL_FROM}"                                >> ${MAIL_TEXT}
echo "Subject: `echo -n "${MAIL_SUB}" | nkf -M`"         >> ${MAIL_TEXT}
echo "X-Mailer: sendmail system"                         >> ${MAIL_TEXT}
echo "Mime-Version: 1.0"                                 >> ${MAIL_TEXT}
echo ""                                                  >> ${MAIL_TEXT}

# body
echo -e -n ${MAIL_BODY}
echo -e -n ${MAIL_BODY} | nkf -MB >> ${MAIL_TEXT}

echo ""
echo ""

# send
#nkf -w --overwrite ${MAIL_TEXT}
cat ${MAIL_TEXT}
cat ${MAIL_TEXT} | sendmail -t

echo ""
echo ""
echo "sendmail done."
