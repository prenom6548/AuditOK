#   Copyright 2024 KylinSoft Co., Ltd.
#
#   This program is free software: you can redistribute it and/or modify it under
#   the terms of the GNU General Public License as published by the Free Software
#   Foundation, either version 3 of the License, or (at your option) any later
#   version.
#
#   This program is distributed in the hope that it will be useful, but WITHOUT
#   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#   FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License along with
#   this program. If not, see <https://www.gnu.org/licenses/>.

import chromadb
from chromadb.config import Settings
import multiprocessing

def do_list_collections(shared_list, path):
    client = chromadb.PersistentClient(path=path)
    results = client.list_collections()
    for collection in results:
        shared_list.append(collection.name)

def do_create_collection(path, collection_name, collection_metadata="cosine"):
    client = chromadb.PersistentClient(path=path)
    client.create_collection(name=collection_name, metadata={"hnsw:space": collection_metadata})

def do_get_or_create_collection(path, collection_name, collection_metadata="cosine"):
    client = chromadb.PersistentClient(path=path)
    client.get_or_create_collection(name=collection_name, metadata={"hnsw:space": collection_metadata})

def do_add_data(path, collection_name, ids,  metadatas, vectors = None, documents = None):
    client = chromadb.PersistentClient(path = path)
    collection = client.get_collection(name = collection_name)
    if documents == [""]:
        documents = None
    if vectors is None:
        vectors = [[1, 1, 1] for _ in range(len(ids))]
    collection.add(
        ids = ids,
        embeddings = vectors,
        documents = documents,
        metadatas = metadatas
    )

def do_get_data(shared_dict, path, collection_name, id=None, source=None):
    client = chromadb.PersistentClient(path = path)
    collection = client.get_collection(name = collection_name)
    if source == "":
        source = None
    filter = None if source is None else {"$contains": source}

    data = collection.get(
        ids=id,
        where_document=filter,
        include=["embeddings", "metadatas", "documents"]
    )
    shared_dict['ids'] = data.get('ids')
    shared_dict['embeddings'] = data.get('embeddings')
    shared_dict['metadatas'] = data.get('metadatas')
    shared_dict['documents'] = data.get('documents')

def do_delete_data(path, collection_name, sourceIds=None, ids=None):
    client = chromadb.PersistentClient(path = path)
    collection = client.get_collection(name = collection_name)
    filter=None
    if sourceIds is not None:
        filter = {"sourceId": {"$in": [str(sourceId) for sourceId in sourceIds if sourceId is not None]}}
    print(filter)
    collection.delete(
        ids=ids,
        where=filter
    )

def do_query_data(shared_dict, path, collection_name, embeddings=None, results=10):
    client = chromadb.PersistentClient(path = path)
    collection = client.get_collection(name = collection_name)
    data = collection.query(
        query_embeddings=embeddings,
        n_results=results
    )
    shared_dict['ids'] = data.get('ids')[0]
    shared_dict['metadatas'] = data.get('metadatas')[0]
    shared_dict['documents'] = data.get('documents')[0]
    shared_dict['distances'] = data.get('distances')[0]

def do_update_data(path, collection_name, ids, documents = None, vectors = None, metadatas = None):
    client = chromadb.PersistentClient(path = path)
    collection = client.get_collection(name = collection_name)
    collection.update(
        ids = ids,
        embeddings = vectors,
        documents = documents,
        metadatas = metadatas
    )

class ChromaDB:
    def __init__(self,  path):
        self.path = path

    def list_collections(self):
        shared_list = multiprocessing.Manager().list()
        p = multiprocessing.Process(target=do_list_collections, args=(shared_list, self.path))
        p.start()
        p.join()
        return list(shared_list)

    #创建collection
    def create_collection(
        self,
        collection_name,
        collection_metadata= "cosine"
    ):
        p = multiprocessing.Process(target=do_create_collection, args=(self.path, collection_name))
        p.start()
        p.join()

    def get_or_create_collection(
        self,
        collection_name,
        collection_metadata="cosine"
    ):
        p = multiprocessing.Process(target=do_get_or_create_collection, args=(self.path, collection_name))
        p.start()
        p.join()

    #增
    def add_data(self, collection_name, ids,  metadatas, vectors = None, documents = None):
        p = multiprocessing.Process(target=do_add_data, args=(self.path, collection_name, ids, metadatas, vectors, documents))
        p.start()
        p.join()

    #获得所有数据
    def get_data(self, collection_name, id=None, source=None):
        shared_dict = multiprocessing.Manager().dict()
        p = multiprocessing.Process(target=do_get_data, args=(shared_dict, self.path, collection_name, id, source))
        p.start()
        p.join()
        return dict(shared_dict)

    #删
    def delete_data(self, collection_name, sourceIds=None, ids=None):
        p = multiprocessing.Process(target=do_delete_data, args=(self.path, collection_name, sourceIds, ids))
        p.start()
        p.join()

    #查
    def query_data(
        self,
        collection_name,
        embeddings=None,
        results=10
    ):
        shared_dict = multiprocessing.Manager().dict()
        p = multiprocessing.Process(target=do_query_data, args=(shared_dict, self.path, collection_name, embeddings, results))
        p.start()
        p.join()
        return dict(shared_dict)

     #更新跟add保持一致
    def update_data(self, collection_name, ids, documents = None, vectors = None, metadatas = None):
        p = multiprocessing.Process(target=do_update_data, args=(self.path, collection_name, ids, documents, vectors, metadatas))
        p.start()
        p.join()

# if "__main__" == __name__:
#     db = ChromaDB("/home/wangyan/chroma/demo-chroma/database6")
#     db.create_collection("collection_demo1")
#     # db.add_data("collection_demo", ["id1","id2"], ["/a", "/b"], None, [{"soure":"/a", "time": 1},{"soure":"/b", "time": 2}])
#     # db.delete_data("collection_demo1", None, ["idd1","idd2"] )
#     db.add_data("collection_demo1", ["idd1","idd2"],[{"soureId":"id1"}, {"soureId":"id2"}], [[1,2,3], [2,3,4]])
#     # results=db.get_data("collection_demo")
#     results= db.get_data("collection_demo1", ["idd1","idd2"])
#     print(results)

# ids = uuid
# collection1 = {metadate: path ,time. id:ids}
# colletion2 = {emdiing: sss ,  metadata: ids, id:dddd}