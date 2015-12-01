#!/usr/bin/python

import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.base import MIMEBase
from email import encoders
from optparse import OptionParser
import re
import os, sys

class Email:

    def __init__(self, host="10.128.161.91", user="test.adm@test.com", passwd=None, debug=False):
        self._msg = MIMEMultipart("mixed")
        self._host = host
        self._user = user
        self._passwd = passwd
        self._subject = ""
        self._text = ""
        self._attachs = []
        self._debug = debug

    def set_subject(self, subject):
        self._subject = subject

    def send(self, from_addr, to_addrs, cc_addrs=[]):
        body = MIMEBase("text", "html", charset="utf-8")
        body.set_payload(self._text)
        self._msg.attach(body)

        for attach in self._attachs:
            self._msg.attach(attach)

        self._msg.add_header("From", from_addr)
        self._msg.add_header("To", ", ".join(list(to_addrs)))
        if cc_addrs:
            self._msg.add_header("CC", ", ".join(list(cc_addrs)))
        self._msg.add_header("Subject", self._subject)

        with open("/tmp/%s.eml" % self._subject, "w+") as f:
            f.write(self.as_string())

        if not self._debug:
            smtp = smtplib.SMTP(self._host)
            if self._passwd:
                smtp.login(self._sender.split("@")[0], self._passwd)
            smtp.sendmail(self._user, list(set(to_addrs)) + list(set(cc_addrs)), self._msg.as_string())
            smtp.quit()

    def add_file(self, path):
        attach = MIMEBase("application", "octet-stream")
        with open(path, "rb") as f:
            attach.set_payload(f.read())
        encoders.encode_base64(attach)
        attach.add_header("content-disposition", "attachment", filename=os.path.basename(path))
        self._attachs.append(attach)

    def add_text(self, text):
        self._text += text

    def as_string(self):
        return self._msg.as_string()

def main():
    parser = OptionParser()
    parser.add_option("--from", dest="from_addr",
            help="who send the mail", default="conn.bsp@test.com")
    parser.add_option("--subject", dest="subject",
            help="the subject of the mail", default="Result of conn-team's auto test")
    parser.add_option("--body", dest="body",
            help="the main content of the mail", default="Auto Test.")
    parser.add_option("--to", dest="to_addrs",
            help="who the mail send to", default="liangxiaoju@126.com")
    parser.add_option("--cc", dest="cc_addrs",
            help="who the mail CC to", default="")
    parser.add_option("--attach", dest="attachment",
            help="attachments for the mail", default="")
   
    (options, args) = parser.parse_args()

    from_addr = options.from_addr
    subject = options.subject
    to_addrs = str(options.to_addrs).replace(",", " ").replace(";", " ").split()
    cc_addrs = str(options.cc_addrs).replace(",", " ").replace(";", " ").split()
    bodymsg = str(options.body)
    attachments = str(options.attachment).replace(",", " ").replace(";", " ").split()

    mail = Email()
    mail.set_subject(subject)
    mail.add_text(bodymsg)

    for attach in attachments:
        if os.path.isfile(attach):
            mail.add_file(attach)

    mail.send(from_addr, to_addrs, cc_addrs)

if __name__ == "__main__":
    main()
