from lxml.etree import ElementTree, parse
zh_source="target/ukui-panel/usr/share/ukui-panel/panel/resources/ukui-panel_zh_CN.ts"
bo_source="ukui-panel_0831_mn.ts"
target="target/ts/bo_CN/ukui-panel_mn.ts"

zh_source_tree = parse(zh_source)
bo_source_tree = parse(bo_source)

for zh_context in zh_source_tree.getroot():
    for zh_message in zh_context:
        for bo_context in bo_source_tree.getroot():
            for bo_message in bo_context:
                if bo_message.find("source") is None or zh_message.find("source") is None:
                    continue
                if bo_message.find("source").text == zh_message.find("source").text:
                    if bo_message.find("translation").text:
                        zh_message.find("translation").text = bo_message.find("translation").text
                        break
                else:
                    zh_message.find("translation").text = ""



#for message in list(zh_source_tree.iter("message")):
#    for bo_message in list(bo_source_tree.iter("message")):
#        if message.find("source").text == bo_message.find("source").text:
#            if bool(bo_message.find("translation").text) is True:
#                message.find("translation").text = bo_message.find("translation").text
#        else:
#            message.find("translation").text = ""

zh_source_tree.getroot().set('language', 'zh_HK')

zh_source_tree.write(str(target), encoding='utf-8', xml_declaration=True)
