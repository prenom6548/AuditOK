import sys
from lxml import etree

def help():
    print('\tpython3 xml_issecount.py -h')
    print('\tpython3 xml_issecount.py basic xxx.xml')
    print('\tpython3 xml_issecount.py patch xxx.xml')

def getxml(path):
    a=open(path).read().encode("utf-8")
    parser=etree.XMLParser(strip_cdata=False)
    a=etree.fromstring(a, parser)
    return a

def printxml(a):
    print(etree.tostring(a,pretty_print=True).decode('UTF-8'))

def xml_issuecount_folder(xml_file):
    a=getxml(xml_file)
    b=a.xpath('//ReportSection[1]//GroupingSection')
    for i in b:
        print(i.xpath('groupTitle/text()')[0],i.xpath('@count')[0])
    b=a.xpath('//ReportSection[2]//SubSection')
    print(b[0].xpath('Text/text()')[0])
    for i in [1,2,3]:
        print(b[i].xpath('IssueListing/Refinement/text()')[0])
        c=b[i].xpath('IssueListing/Chart/GroupingSection')
        for j in c:
            print('\t',j.xpath('groupTitle/text()')[0],j.xpath('@count')[0])

def xml_issuecount_patch(xml_file):
    new_count=-1
    a=getxml(xml_file)
    b=a.xpath('//ReportSection[1]//GroupingSection')
    for i in b:
        print(i.xpath('groupTitle/text()')[0],i.xpath('@count')[0])
        if "Issue New" in i.xpath('groupTitle/text()')[0]:
            new_count=i.xpath('@count')[0]
    print("new_count",new_count)
    return new_count


if sys.argv[1] == '-h':
    help()
elif sys.argv[1] == 'basic':
    xml_issuecount_folder(sys.argv[2])
elif sys.argv[1] == 'patch':
    xml_issuecount_patch(sys.argv[2])



