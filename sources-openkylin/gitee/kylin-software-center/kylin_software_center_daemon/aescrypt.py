from Cryptodome.Cipher import AES
from Cryptodome import Random
from Cryptodome.Hash import SHA
from Cryptodome.Cipher import PKCS1_v1_5 as Cipher_pkcs1_v1_5
from Cryptodome.Signature import PKCS1_v1_5 as Signature_pkcs1_v1_5
from Cryptodome.PublicKey import RSA
import base64

class Aescrypt():
    def __init__(self,key = None,model = None,iv = None):
        if(key != None):
            self.key = self.add_16(key)
        else:
            self.key = key
        self.model = model
        self.iv = iv

    def add_16(self,par):
        if type(par) == str:
            par = par.encode()
        while len(par) % 16 != 0:
            par += b'\x00'
        return par

    def aesencrypt(self,text):
        text = self.add_16(text)
        if self.model == AES.MODE_CBC:
            self.aes = AES.new(self.key,self.model,self.iv)
        elif self.model == AES.MODE_ECB:
            self.aes = AES.new(self.key,self.model)
        self.encrypt_text = self.aes.encrypt(text)
        return self.encrypt_text

    def aesdecrypt(self,text):
        if self.model == AES.MODE_CBC:
            self.aes = AES.new(self.key,self.model,self.iv)
        elif self.model == AES.MODE_ECB:
            self.aes = AES.new(self.key,self.model)
        self.decrypt_text = self.aes.decrypt(text)
        self.decrypt_text = self.decrypt_text.strip(b"\x00")
        return self.decrypt_text

    #msg:byte 类型
    def add_key_encrypt(self,key,msg):
        rsakey = RSA.importKey(key)
        signer = Signature_pkcs1_v1_5.new(rsakey)
        digest = SHA.new()
        digest.update(msg)
        sign = signer.sign(digest)
        signature = base64.b64encode(sign)
        return signature

    #msg：为生产的加签了的数据，msg1为原始数据（均为byte类型）
    def add_key_dncrypt(self,key,msg,msg1):
        rsakey = RSA.importKey(key)
        verifier = Signature_pkcs1_v1_5.new(rsakey)
        digest = SHA.new()
        digest.update(msg1)
        is_verify = verifier.verify(digest, base64.b64decode(msg))
        return is_verify
