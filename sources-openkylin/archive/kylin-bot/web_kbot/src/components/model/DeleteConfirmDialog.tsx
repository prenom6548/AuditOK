import { ModelConfirmDialog } from './ModelConfirmDialog';

interface DeleteConfirmDialogProps {
  open: boolean;
  onCancel: () => void;
  onConfirm: () => Promise<void> | void;
}

export function DeleteConfirmDialog({
  open,
  onCancel,
  onConfirm,
}: DeleteConfirmDialogProps) {
  return (
    <ModelConfirmDialog
      open={open}
      ariaLabel="删除后将无法恢复，是否确认删除该模型？"
      dialogHeight={172}
      message="删除后将无法恢复，是否确认删除该模型？"
      confirmLabel="删除"
      iconSrc="icons/dialog-question.svg"
      onCancel={onCancel}
      onConfirm={onConfirm}
    />
  );
}
