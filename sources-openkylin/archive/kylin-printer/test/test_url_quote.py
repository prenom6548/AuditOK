from urllib.parse import quote

def urlquote (x):
    q = quote (x)
    for c in ["/", "@", ":"]:
        q = q.replace (c, "%%%02X" % ord (c))

    return q

def _construct (group, host, share, user='', password=''):
    uri_password = ''
    if password:
        uri_password = ':' + urlquote (password)
    if user:
        uri_password += '@'
    uri = "%s%s%s" % (urlquote (user),
                        uri_password,
                        urlquote (group))
    if len (group) > 0:
        uri += '/'
    uri += urlquote (host)
    if len (share) > 0:
        uri += "/" + urlquote (share)
    return uri



print(_construct("group", "local@:host", "test", "kylin", "123"))


