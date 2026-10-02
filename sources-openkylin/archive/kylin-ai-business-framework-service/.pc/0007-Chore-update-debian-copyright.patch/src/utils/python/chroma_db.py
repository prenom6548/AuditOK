import chromadb
from chromadb.config import Settings

class ChromaDB:
    def __init__(self,  pathname):
        self.client = self.create_PersistentClient(pathname)

    #创建本地存储数据的client链接，入参allow表示是否允许PersistentClient重置其状态或数据库，此处默认不允许；
    def create_PersistentClient(self, pathname, allow=False):
        chroma_client = chromadb.PersistentClient(path=pathname, settings=Settings(allow_reset=allow))
        return chroma_client

    def list_collections(self):
        results = self.client.list_collections()
        names = [collection.name for collection in results]
        print(names)
        return names

    #创建collection
    def create_collection(
        self,
        collection_name,
        collection_metadata= "cosine"
    ):
        self.client.create_collection(name=collection_name, metadata={"hnsw:space": collection_metadata})

    #获取collection
    def get_collection(
        self,
        collection_name,
    ):
        collection = self.client.get_collection(name=collection_name)
        return collection

    def get_or_create_collection(
        self,
        collection_name,
        collection_metadata="cosine"
    ):
        self.client.get_or_create_collection(name=collection_name, metadata={"hnsw:space": collection_metadata})

    #增
    def add_data(self, collection_name, ids,  metadatas, vectors = None, documents = None):
        collection = self.get_collection(collection_name)
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

    #获得所有数据
    def get_data(self, collection_name, id=None, source=None):
        collection = self.get_collection(collection_name)
        if source == "":
            source = None
        filter = None if source is None else {"$contains": source}

        data = collection.get(
            ids=id,
            # embeddings=vectors,
            where_document=filter,
            include=["embeddings", "metadatas", "documents"]
        )
        results = {'ids': data.get('ids'), 'embeddings': data.get('embeddings'), 'metadatas': data.get('metadatas'), 'documents': data.get('documents')}
        return results

    #删
    def delete_data(self, collection_name, sourceIds=None, ids=None):
        collection = self.get_collection(collection_name)
        filter=None
        if sourceIds is not None:
            filter = {"sourceId": {"$in": [str(sourceId) for sourceId in sourceIds if sourceId is not None]}}
        print(filter)
        collection.delete(
            ids=ids,
            where=filter
        )

    #查
    def query_data(
        self,
        collection_name,
        embuddings=None,
        results=10
    ):
        collection = self.get_collection(collection_name)
        data=collection.query(
            query_embeddings=embuddings,
            n_results=results
        )
        results = {'ids': data.get('ids')[0],'metadatas': data.get('metadatas')[0], 'documents': data.get('documents')[0], 'distances': data.get('distances')[0]}
        return results

     #更新跟add保持一致
    def update_data(self, collection_name, ids, documents = None, vectors = None, metadatas = None):
        collection = self.get_collection(collection_name)
        collection.update(
            ids = ids,
            embeddings = vectors,
            documents = documents,
            metadatas = metadatas
        )

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