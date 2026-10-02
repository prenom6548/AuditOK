TEMPLATE = subdirs

# 定义子目录
SUBDIRS += mydefine \
           utils \
           mydusizetool 


# 设置子目录间的依赖关系，确保构建顺序正确
# mydefine作为基础库，其他模块可能依赖它
mydefine.file = mydefine/mydefine-test.pro
utils.file = utils/utils-test.pro
mydusizetool.file = mydusizetool/mydusizetool-test.pro



