import ctypes
from ctypes.util import find_library
from PyQt5.QtWidgets import QApplication, QMainWindow, QComboBox, QLineEdit, QPushButton, QVBoxLayout, QWidget

# 定义C结构体
class SecurityQuestion(ctypes.Structure):
    _fields_ = [("questionId", ctypes.c_int),
                 ("questionText", ctypes.c_char * 128)]

class SecurityQuestionListNode(ctypes.Structure):
    pass

SecurityQuestionListNode._fields_ = [("question", SecurityQuestion),
                 ("next", ctypes.POINTER(SecurityQuestionListNode))]

class SecurityAnswer(ctypes.Structure):
    _fields_ = [("userId", ctypes.c_int),
                 ("questionId", ctypes.c_int),
                 ("encryptedAnswer", ctypes.c_char * 65)]

class SecurityAnswerListNode(ctypes.Structure):
    pass

SecurityAnswerListNode._fields_ = [("answer", SecurityAnswer),
                 ("next", ctypes.POINTER(SecurityAnswerListNode))]

# 加载动态库
libsecurity = ctypes.CDLL(find_library('kba'))

# 函数接口
getPresetSecurityQuestions = libsecurity.getPresetSecurityQuestions
getPresetSecurityQuestions.argtypes = [ctypes.POINTER(ctypes.c_int)]
getPresetSecurityQuestions.restype = ctypes.POINTER(SecurityQuestionListNode)

setUserSecurityAnswers = libsecurity.setUserSecurityAnswers
setUserSecurityAnswers.argtypes = [ctypes.POINTER(SecurityAnswerListNode)]
setUserSecurityAnswers.restype = ctypes.c_int

# 加密函数接口
KPR_PKCS5_PBKDF2_HMAC2 = libsecurity.KPR_PKCS5_PBKDF2_HMAC2
KPR_PKCS5_PBKDF2_HMAC2.argtypes = [ctypes.c_char_p, ctypes.c_size_t, ctypes.c_char_p, ctypes.c_size_t, ctypes.c_ulong, ctypes.c_ulong, ctypes.c_char_p]
KPR_PKCS5_PBKDF2_HMAC2.restype = ctypes.c_int

# 十六进制转换函数接口
Hex = libsecurity.Hex
Hex.argtypes = [ctypes.c_char_p, ctypes.c_int, ctypes.c_int]
Hex.restype = ctypes.c_char_p

class PasswordRecovery(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()

    def initUI(self):
        self.setWindowTitle('找回密码')

        # 获取预设的安全问题列表
        count_ptr = ctypes.c_int()
        questions_list = getPresetSecurityQuestions(ctypes.byref(count_ptr))
        self.questions = self.populateQuestions(questions_list, count_ptr.value)

        # 创建下拉框用于选择安全问题
        self.questionComboBox = QComboBox(self)
        for question in self.questions:
            self.questionComboBox.addItem(question)
        
        self.questionComboBox2 = QComboBox(self)
        for question in self.questions:
            self.questionComboBox2.addItem(question)
        
        self.questionComboBox3 = QComboBox(self)
        for question in self.questions:
            self.questionComboBox3.addItem(question)

        # 创建答案输入框
        self.answerLineEdit = QLineEdit(self)
        self.answerLineEdit2 = QLineEdit(self)
        self.answerLineEdit3 = QLineEdit(self)

        # 创建提交按钮
        self.submitButton = QPushButton('提交', self)
        self.submitButton.clicked.connect(self.verifyAnswer)

        # 创建布局并添加控件
        layout = QVBoxLayout()
        layout.addWidget(self.questionComboBox)
        layout.addWidget(self.answerLineEdit)
        layout.addWidget(self.questionComboBox2)
        layout.addWidget(self.answerLineEdit2)
        layout.addWidget(self.questionComboBox3)
        layout.addWidget(self.answerLineEdit3)
        layout.addWidget(self.submitButton)

        centralWidget = QWidget()
        centralWidget.setLayout(layout)
        self.setCentralWidget(centralWidget)

    def populateQuestions(self, head, count):
        self.questions = []
        current = head
        while count > 0 and current is not None:
            question_text = current.contents.question.questionText.decode('utf-8')
            self.questions.append(question_text)
            current = current.contents.next
            count -= 1
        return self.questions
    
def createAnswerList(self):
    answers = []
    for i in range(3):
        question_id = self.questionComboBox.itemData(i, QtCore.Qt.UserRole).toInt()[0]  # 假设问题ID存储在下拉框的UserRole中
        answer_text = self.getAnswerLineEditText(i)
        if answer_text:
            # 加密答案
            encrypted_answer = self.encryptAnswer(answer_text)
            answer = SecurityAnswer(1000, question_id, encrypted_answer)
            answers.append(answer)
    return answers

def encryptAnswer(self, answer_text):
    # 假设salt和iteration_count等参数已经定义
    salt = b'some_salt'
    iteration_count = 10000
    key_length = 32  # 根据需要调整密钥长度

    # 分配输出缓冲区
    output = ctypes.create_string_buffer(key_length)

    # 调用加密函数
    result = KPR_PKCS5_PBKDF2_HMAC2(answer_text.encode('utf-8'), len(answer_text), salt, len(salt), iteration_count, key_length, output)

    if result != 0:
        raise Exception('加密失败')

    # 将加密后的答案转换为十六进制字符串
    hex_output = Hex(output, key_length, True).decode('utf-8')
    return hex_output

    def getAnswerLineEditText(self, index):
        if index == 0:
            return self.answerLineEdit.text()
        elif index == 1:
            return self.answerLineEdit2.text()
        elif index == 2:
            return self.answerLineEdit3.text()

    def createAnswerChain(self, answers):
        # 创建答案链表
        head = None
        current = None
        for answer in reversed(answers):
            new_node = SecurityAnswerListNode(answer, None)
            if current is None:
                head = new_node
                current = head
            else:
                current.next = new_node
                current = new_node
        return head

    def bindAnswers(self):
        answers = self.createAnswerList()
        if not answers:
            QMessageBox.warning(self, '警告', '请填写所有答案。')
            return
        answer_chain = self.createAnswerChain(answers)
        # 假设用户ID为12345，这里需要根据实际情况来设置
        userId = 12345
        result = setUserSecurityAnswers(answer_chain, userId)
        if result == 0:
            QMessageBox.information(self, '成功', '答案绑定成功。')
        else:
            QMessageBox.warning(self, '警告', '答案绑定失败。')


    def verifyAnswer(self):
        selected_question = self.questionComboBox.currentText()
        entered_answer = self.answerLineEdit.text()

        # 这里应该有验证答案的逻辑
        # 为了演示，我们假设验证总是成功
        QMessageBox.information(self, '成功', f'验证成功，问题：{selected_question}，答案：{entered_answer[:10] + "****" if len(entered_answer) > 10 else entered_answer}')

if __name__ == '__main__':
    app = QApplication([])
    window = PasswordRecovery()
    window.show()
    app.exec_()