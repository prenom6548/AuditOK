import { listen, type UnlistenFn } from "@tauri-apps/api-v1/event";

type DragDropEvent = { payload: { type: string; paths?: string[] } };
type DragDropHandler = (event: DragDropEvent) => void;

export function getCurrentWebview() {
  return {
    async onDragDropEvent(handler: DragDropHandler): Promise<UnlistenFn> {
      const unlistenOver = await listen("tauri://file-drop-hover", () =>
        handler({ payload: { type: "over" } })
      );
      const unlistenLeave = await listen("tauri://file-drop-cancelled", () =>
        handler({ payload: { type: "leave" } })
      );
      const unlistenDrop = await listen<string[]>("tauri://file-drop", (event) =>
        handler({ payload: { type: "drop", paths: event.payload } })
      );
      return () => {
        unlistenOver();
        unlistenLeave();
        unlistenDrop();
      };
    }
  };
}
