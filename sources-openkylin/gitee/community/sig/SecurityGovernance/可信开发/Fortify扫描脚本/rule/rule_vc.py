# 首先需要拷贝<RuleDefinitions>节点出来

from lxml import etree

def getxml(path):
    with open(path,'r') as f:
        a=f.read().encode('UTF-8')
        parser=etree.XMLParser(strip_cdata=False)
        a=etree.fromstring(a, parser)
    return a

def printxml(a):
    print(etree.tostring(a,pretty_print=True).decode('UTF-8'))

def rule_sec_1(raw_file):
    with open(raw_file,'r') as raw_file_open: 
        flag=0
        for i in range(100):
            a=raw_file_open.readline()
            if 'RuleDefinitions' in a:
                flag=1
            if flag == 1:
                break
            print(a,end='')

def rule_sec_2(raw_file,ruledef_file):
    with open(raw_file,'r') as raw_file_open: 
        with open(ruledef_file,'w') as ruledef_file_open:
            flag=0
            while 1:   
                a=raw_file_open.readline()
                if 'RuleDefinitions' in a:
                    flag=1
                if flag == 1:
                    ruledef_file_open.write(a)
                if flag == 2:
                    break
                if '/RuleDefinitions' in a:
                    flag=2

def rule_sec_3(raw_file):
    with open(raw_file,'r') as raw_file_open: 
        flag=0
        while 1:   
            a=raw_file_open.readline()
            if a == '':
                break
            if flag == 1:
                print(a,end='')
            if '/RuleDefinitions' in a:
                flag=1

def add_KL_id(a,vcs,ids,KL_rule):   
    b=a.xpath('//VulnCategory/..')
    for c in b:
        vc=c.xpath('VulnCategory')[0].text
        for i in ids:
            if vcs[i] in vc:
                c.xpath('VulnCategory')[0].text='KL Rule '+KL_rule[i]+'; '+vc
    return a 

def add_MISRA_id(a):
    rules=['1.3',		
        '2.1',
        '9.1',
        '10.3',
        '12.4',	
        '13.4',	
        '14.4',	
        '17.7',
        '21.3',
        '21.5',	
        '22.1',	
        '22.2']
    for rule in rules:
        b=a.xpath('//Group[@name="altcategoryMISRAC2012" and text()="Rule '+rule+'"]/../..')
        for c in b:
            vc=c.xpath('VulnCategory/text()')[0]
            c.xpath('VulnCategory')[0].text='MISRA Rule '+rule+'; '+vc
    return a

def remov_vc(a):
    b=a.xpath('//VulnCategory/..')
    for c in b:
        vc=c.xpath('VulnCategory')[0].text
        # if not 'KL' in vc and not 'MISRA' in vc:
        if not 'KL' in vc:
            c.getparent().remove(c)  
    return a

raw_file='core_cpp_2022_2.xml'
ruledef_file='ruledef.xml'
vcs=["Memory Leak", "Command Injection", "Path Manipulation", "Buffer Overflow", "Code Correctness", "Constraint", "Dangerous Function", "Dead Code", "Denial of Service", "Directory Restriction", "Double Free", "Format String", "Fortify Internal", "Heap Inspection", "Illegal Pointer Value", "Insecure Compiler Optimization", "Insecure Randomness", "Insecure Temporary File", "Integer Overflow", "Key Management", "Least Privilege Violation", "Log Forging", "Missing Check against Null", "Null Dereference", "Obsolete", "Often Misused", "Out-of-Bounds Read", "Password Management", "Poor Style", "Portability Flaw", "Privacy Violation", "Process Control", "Race Condition", "Redundant Null Check", "Resource Injection", "Setting Manipulation", "String Termination Error", "System Information Leak", "Type Mismatch", "Unchecked Return Value", "Undefined Behavior", "Uninitialized Variable", "Unreleased Resource", "Unsafe Reflection", "Use After Free", "Weak Cryptographic Hash"]
KL_rule=["4.1", "6.1", "5.1"]
rule_sec_1(raw_file)
rule_sec_2(raw_file,ruledef_file)
a=getxml(ruledef_file)
#a=add_MISRA_id(a)
a=add_KL_id(a,vcs,[0,1,2],KL_rule)
a=remov_vc(a)
printxml(a)
rule_sec_3(raw_file) 




