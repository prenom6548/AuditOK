import cups

ppd='driverless:ipp://AABBCCDD._ipp._tcp.local/'
# ppd='driverless:ipp://AABBCCDD._ipp._tcp.local/'


conn = cups.Connection()

f = conn.getServerPPD(ppd)

print(f)