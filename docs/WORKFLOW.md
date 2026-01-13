# CI/CD Workflow

This document explains the rack-controller CI/CD pipeline, including the AI-assisted development loop and manual deployment gates.

## Pipeline Overview

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  AI/Human   │────▶│    Build    │────▶│  HIL Tests  │────▶│   Review    │
│  Proposes   │     │   (Auto)    │     │   (Auto)    │     │  (Manual)   │
│   Changes   │     │             │     │             │     │             │
└─────────────┘     └─────────────┘     └─────────────┘     └─────────────┘
                                              │
                                              ▼ (on failure)
                                        ┌─────────────┐
                                        │   Create    │
                                        │   Issue     │
                                        │  (Auto)     │
                                        └─────────────┘
                                              │
                                              ▼
                                        ┌─────────────┐
                                        │  AI Reads   │
                                        │  Issue &    │
                                        │   Fixes     │
                                        └─────────────┘
                                              │
                                              ▼
                                        ┌─────────────┐
                                        │   Deploy    │
                                        │  (Manual)   │
                                        └─────────────┘
```

## Workflow Stages

### 1. Code Changes (Human or AI)

Changes can be proposed by:
- **Human developers** via normal PR workflow
- **AI assistants** (Claude, Copilot, etc.) proposing fixes

All changes go through the same CI pipeline.

### 2. Build (`build.yml`)

**Trigger:** Push to `main`, Pull Requests

**Runner:** ZimaBlade #1 (build server)

**Jobs:**
- `pio_build_s3`: Build ESP32-S3 firmware
- `pio_build_cyd`: Build CYD firmware  
- `idf_build_p4`: Build ESP32-P4 firmware (disabled until hardware available)

**Artifacts:** Compiled `.bin` and `.elf` files

**Failure Mode:** PR blocked, developer notified

### 3. HIL Tests (`hil.yml`)

**Trigger:** Pull Requests only

**Runner:** ZimaBlade #2 (HIL server with device access)

**Jobs:**
- `hil_flash_s3`: Flash S3 test device, validate boot contract
- `hil_flash_cyd`: Flash CYD test device, validate boot contract
- `hil_flash_p4`: Flash P4 test device (disabled)

**Validation Steps:**
1. Build firmware
2. Flash to physical test device
3. Monitor serial for boot contract JSON (10s timeout)
4. Subscribe to MQTT and verify status message

**Artifacts:** Logs from failed tests

### 4. Auto-Issue Creation (`issue_on_fail.yml`)

**Trigger:** HIL workflow completes with failure

**Runner:** GitHub-hosted (ubuntu-latest)

**Creates Issue With:**
- Link to failed workflow run
- Commit SHA
- List of failed jobs
- Instructions for AI to investigate and fix
- Labels: `ci-failure`, `ai-fix-needed`

### 5. AI Fix Loop

When an issue is created:

1. AI (or human) reads the issue
2. Downloads and analyzes failure logs
3. Identifies root cause
4. Proposes fix via new PR
5. References the issue in PR description
6. CI runs again on the fix PR
7. If passing, issue can be closed

### 6. Manual Deployment (`release_flash.yml`)

**Trigger:** Manual dispatch only (`workflow_dispatch`)

**Runner:** ZimaBlade #2 (flash server)

**Inputs:**
- `target`: s3, cyd, or p4
- `device_port`: Serial port path
- `environment`: dev or release
- `run_validation`: Whether to run post-flash checks

**Purpose:** Flash staging/production devices after PR is merged

**Gate:** Requires explicit human action—no automatic production deployment

## Boot Contract

All firmware must implement the boot contract for HIL validation:

### Serial Output (within 10 seconds)

```json
{"device":"<mac-id>","fw":"<git-sha>","target":"<s3|cyd|p4>","selftest":"pass"}
```

On failure:
```json
{"device":"<mac-id>","fw":"<git-sha>","target":"<s3|cyd|p4>","selftest":"fail","err":"<code>"}
```

### MQTT Status (retained)

**Topic:** `lab/<device-id>/status`

**Payload:**
```json
{
  "device": "<mac-id>",
  "fw": "<git-sha>",
  "target": "<s3|cyd|p4>",
  "selftest": "pass|fail",
  "ts": <epoch-seconds>
}
```

## AI Integration Points

### For Claude/AI Assistants

When working on this repository:

1. **Check CI status** before making changes
2. **Read open issues** with `ci-failure` label
3. **Analyze failure logs** from workflow artifacts
4. **Propose targeted fixes** via PR
5. **Reference issues** in PR description: `Fixes #123`

### Common Failure Patterns

| Symptom | Likely Cause | Fix |
|---------|--------------|-----|
| Build fails | Syntax error, missing include | Check compiler output |
| Flash fails | Port not found | Check udev rules, device connection |
| Serial timeout | Boot contract not printed in time | Optimize startup, check WiFi timeout |
| MQTT timeout | Network issue, broker down | Check connectivity, broker status |
| Selftest fail | Hardware issue, sensor disconnected | Check device hardware |

### Issue Template for AI

When creating issues manually for AI to fix:

```markdown
## Problem
[Describe what's failing]

## Error Output
[Paste relevant logs]

## Expected Behavior
[What should happen]

## Files to Check
- `firmware/xxx/src/main.cpp`
- `tools/serial_watch.py`

## Constraints
- Must maintain boot contract format
- Serial output within 10 seconds
- Don't break other targets
```

## Manual Gates

The following actions require human intervention:

1. **Merge PR** - Human must approve and merge
2. **Deploy to staging** - Manual `release_flash.yml` dispatch
3. **Deploy to production** - Out of band, not in this pipeline

This ensures no code reaches physical devices without human review.

## Monitoring

### Dashboard Recommendations

Consider setting up:
- GitHub Actions status dashboard
- MQTT broker monitoring
- Device online/offline alerts via `lab/+/status` topic

### Alerts

Configure GitHub notifications for:
- Failed workflows
- New issues with `ci-failure` label
- PRs awaiting review

## Extending the Pipeline

### Adding New Targets

1. Create firmware directory under `firmware/`
2. Add build job to `build.yml`
3. Add HIL job to `hil.yml`
4. Add device to `DEVICE_POOL.md`
5. Create udev rule for stable port
6. Update `release_flash.yml` with new target option

### Adding New Validation Steps

1. Create validation script in `tools/`
2. Add step to `hil.yml` jobs
3. Update failure analysis in `issue_on_fail.yml`
4. Document in this file
