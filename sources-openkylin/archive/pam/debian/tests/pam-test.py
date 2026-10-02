#!/usr/bin/python3
# Copyright 2023, Sam Hartman
# This code may be redistributed under the same terms as Linux Pam
# itself, or at your pution, under the GNU General Public License,
# version 3. 


import PAM

def conversation(auth, queries, userdata):
    results = []
    for prompt, type in queries:
        if type == PAM.PAM_PROMPT_ECHO_OFF:
            results.append(('ThisLongPasswordIsHardCoded', 0))
        else: results.append(('',0))
    return results
# set a password

auth = PAM.pam()
auth.start('passwd')
auth.set_item(PAM.PAM_USER, 'pam_test')
auth.set_item(PAM.PAM_CONV, conversation)
auth.chauthtok()

# Now authenticate and session
auth = PAM.pam()
auth.start('login')
auth.set_item(PAM.PAM_USER, 'pam_test')
auth.set_item(PAM.PAM_CONV, conversation)
auth.authenticate()
auth.acct_mgmt()
auth.open_session()
auth.close_session()
