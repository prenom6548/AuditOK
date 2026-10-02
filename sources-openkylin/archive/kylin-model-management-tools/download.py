import sys
import argparse
from modelscope.hub.snapshot_download import snapshot_download
from modelscope.hub.file_download import model_file_download

# parse command line arguments
# python3 download.py qwen/Qwen1.5-4B-Chat-GGUF --cache-dir=/yds/models/modelscope --file-path=qwen1_5-4b-chat-q5_k_m.gguf

def download_model():
    parser = argparse.ArgumentParser(description="Download model from specified source.")
    
    parser.add_argument("model_id", type=str, help="The ID of the model to download.")
    parser.add_argument("--cache-dir", type=str, help="Directory to cache downloaded models.")
    parser.add_argument("--file-path", type=str, help="The file path of the model file to download.")

    args = parser.parse_args()
    
    # print(f"Model ID: {args.model_id}")
    # print(f"Cache Directory: {args.cache_dir}")
    if args.file_path:
        # print(f"File Path: {args.file_path}")
        return model_file_download(model_id=args.model_id, file_path=args.file_path, cache_dir=args.cache_dir)
    else:
        return snapshot_download(args.model_id, cache_dir=args.cache_dir)

# model_dir = snapshot_download(model_id, cache_dir="/yds/models/modelscope")
# model_dir = model_file_download(model_id='qwen/Qwen1.5-4B-Chat-GGUF',file_path='qwen1_5-4b-chat-q5_k_m.gguf',cache_dir='/yds/models/modelscope')

if __name__ == "__main__":
    model_dir = download_model()
    print(f"Model directory: {model_dir}")
    sys.stdout.flush()
    sys.exit(0)
