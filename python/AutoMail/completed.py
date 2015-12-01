#!/usr/bin/python

from record import *
from table import *
from mail import *
import os
import sys
import shutil
import tempfile
import time

mail_addrs = (
#
#   from_addr,
#   (to_addrs),
#   (cc_addrs)
#

    "auto.mail@test.com",
    (
    ),
    (
    ),
)

def post_result(mail, team):
    db = Record()

    # get new ids first
    ids = db.query_new_records(team, ["id", ], update=1)
    ids = [ i[0] for i in ids ]

    # get new records
    header = db.get_table_header(team)
    for s in ("id", "new", "attach_content"):
        header.remove(s)
    new_records = []
    for i in ids:
        new_records.append(db.query_record(team, i, header))

    # generate summary
    tbl = HtmlTable(team, header, (10, 15, 15, 10, 30, 10, 15))
    for r in new_records:
        tbl.insert_result(r)
    html = tbl.toHTML()

    # generate attachment
    attachments = []
    for i in ids:
        attachments.append(
                db.query_record(
                    team, i, ["attach_name", "attach_content"]))

    attach_dir = tempfile.mkdtemp()

    for fname,attach in attachments:
        if fname and attach:
            with open(os.path.join(attach_dir, fname), "wb") as f:
                f.write(attach)

    # mail

    # add body text
    mail.add_text(html)

    # add attachments
    for fname,attach in attachments:
        if fname and attach:
            path = os.path.join(attach_dir, fname)
            if os.path.isfile(path):
                mail.add_file(path)

    shutil.rmtree(attach_dir)


def main():

    project="Pixi3554GEVDO"
    version="xxxx"

    if len(sys.argv) == 3:
        project = sys.argv[1]
        version = sys.argv[2]

    db = Record()
    teams = db.get_tables()

    mail = Email(debug=False)

    subject = "%s_%s AUTO BSP STRESS TEST Daily Reprot" % (project, version)
    mail.set_subject(subject)

    for team in teams:
        post_result(mail, team)

    from_addr = mail_addrs[0]
    to_addrs = mail_addrs[1]
    cc_addrs = mail_addrs[2]

    try:
        mail.send(from_addr, to_addrs, cc_addrs)
    except:
        print "Failed to send mail !"

    print 'auto email send at %s Completed' % time.strftime('%Y.%m.%d %H:%M:%S',time.localtime(time.time()))



if __name__ == "__main__":
    main()
