# Realtime Version Notes

## Current Stable Entry

- `run_motionbert_realtime.bat`
- Uses `realtime_motionbert.py`
- Status: current rollback target / recommended default
- Notes:
  - correct pose semantics
  - compact local-position transport
  - UE latest-packet-first receive path
  - no-display render skip optimization

## Experimental Pipeline Entry

- `run_motionbert_realtime_pipeline_experimental.bat`
- Uses `versions/realtime/realtime_motionbert_pipeline_experimental.py`
- Status: archived experimental version
- Notes:
  - split `pose2d_loop` and `pose3d_loop`
  - higher parallelism attempt
  - user feedback: slower than previous stable version on current machine

## Snapshots

- `versions/realtime/realtime_motionbert_transport_stable.py`
  - snapshot of the current stable realtime script
- `versions/realtime/realtime_motionbert_pipeline_experimental.py`
  - snapshot of the pipeline experiment before rollback
