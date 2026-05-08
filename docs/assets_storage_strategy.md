# MotionBERT Asset Storage Strategy

This document defines how to manage large non-git files for this repo so that:

- source code stays in `git`
- environments are rebuilt by deploy scripts
- big runtime assets are restored by download or manual copy

## Goal

Split the project into three layers:

1. `git layer`
2. `deploy-download layer`
3. `manual-asset layer`

That gives you reproducible machines without committing Python envs, caches, or giant binaries into the repo.

## Recommended storage layout

### 1. Git layer

Keep these in `git`:

- `.py`
- `.bat`
- `.ps1`
- `.yaml`
- `.md`
- `deploy/requirements-*.txt`
- deployment scripts
- runtime entry scripts
- UE integration scripts

Do not keep these in `git`:

- `.local/`
- `.cache/`
- `alpha_cache/`
- `outputs/`
- large checkpoints
- datasets
- temporary exports

### 2. Deploy-download layer

Use this layer for assets that are large but safe to fetch automatically during deploy.

Best fit:

- public MotionBERT checkpoints
- public AlphaPose weights
- public detector weights
- small to medium demo packs
- your own private checkpoints if you host them behind stable direct links

Recommended hosts:

- Hugging Face Hub for public model weights: <https://huggingface.co/docs/huggingface_hub/en/guides/upload>
- S3-compatible object storage for private or mixed assets:
  - AWS S3: <https://docs.aws.amazon.com/AmazonS3/latest/userguide/Welcome.html>
  - Cloudflare R2: <https://developers.cloudflare.com/r2/get-started/s3/>
  - Backblaze B2 S3 API: <https://www.backblaze.com/docs/cloud-storage-s3-compatible-api>

Optional but less ideal for this repo:

- GitHub Releases for smaller public release bundles: <https://docs.github.com/github/managing-large-files>
- Git LFS for a small number of version-bound binaries: <https://docs.github.com/github/managing-large-files/versioning-large-files/about-git-large-file-storage>

## 3. Manual-asset layer

Keep these out of automatic public download unless you explicitly host them yourself:

- licensed datasets
- Human3.6M-derived data that may require registration or approval
- private training outputs
- sensitive client files
- internal captures

These should be handled by one of:

- manual copy to a documented target path
- private object storage with signed URL
- LAN share
- separate asset bundle distributed inside your team

## Recommended classification for this repo

### Public-download candidates

- `checkpoint/pose3d/FT_MB_lite_MB_ft_h36m_global_lite/best_epoch.bin`
- `.external/AlphaPose/detector/yolox/data/yolox_x.pth`
- any public official MotionBERT release checkpoint
- any public official AlphaPose release checkpoint

### Private-download candidates

- `checkpoint/pose3d/MB_train_h36m_global_5ep/best_epoch.bin`
- your own fine-tuned checkpoints
- your own UE demo packs
- private workflow resources

### Manual-only candidates

- full training datasets
- files with redistribution restrictions
- any asset whose upstream requires login, registration, or license acceptance

## Suggested host assignment

For your current workflow, this is the cleanest split:

### Hugging Face Hub

Use for:

- public MotionBERT checkpoints
- your publicly shareable model weights

Why:

- very natural for AI model files
- stable direct download pattern
- good for versioned model artifacts

### S3-compatible storage

Use for:

- your private checkpoints
- zip bundles
- internal runtime resources
- anything not suitable for public hosting

Why:

- stable direct links
- can do signed URLs
- easy to script in PowerShell deploy

## Deploy integration design

The repo now has:

- [deploy/assets_manifest.template.json](/d:/Git/Github/MotionBERT/deploy/assets_manifest.template.json)

This manifest should become the source of truth for non-git assets.

Recommended next evolution:

1. Fill each asset with a real host URL
2. Add optional checksum fields later if you want integrity verification
3. Teach `deploy_windows_5070ti.ps1` and `deploy_windows_5090.ps1` to read the manifest
4. Download or extract assets by category
5. Leave `manual-only` entries as warnings instead of hard failure

## Practical rule set

Use these rules when deciding where a file goes:

### Put it in git when:

- it is text source
- it is small
- it changes together with code
- it benefits from diff and history

### Put it in deploy-download when:

- it is large
- it can be fetched from a stable URL
- it is legal to redistribute from that URL
- it is needed on every machine

### Put it in manual-only when:

- the upstream requires agreement or login
- it is private
- it is too large or too sensitive for public hosting

## Best practice for your repo

The long-term clean setup is:

1. `git clone` the repo
2. run `deploy_windows_5070ti.bat` or `deploy_windows_5090.bat`
3. let deploy fetch all public and private-link assets from the manifest
4. only manually copy the assets that are truly license-restricted

That gets you close to one-click rebuild without bloating the repo.
