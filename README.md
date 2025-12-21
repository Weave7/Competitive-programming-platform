## ⚠️ Limitations & Deployment Notes

### 🧪 Testing Status

> **Important:**  
This project currently **does not include real or comprehensive test cases**.

- No automated unit tests or integration tests are implemented yet
- Submission evaluation logic is **not battle-tested**
- Docker setup has only been validated for **local development**
- Edge cases (timeouts, memory limits, malicious submissions) are **not fully handled**

⚠️ **This means the platform is not production-ready** and should be treated as an experimental or educational project.

---

## 🐳 Docker Deployment Options

The project includes a `Dockerfile` and `docker-compose.yml` mainly intended for **local development and experimentation**.

### ✅ Supported / Recommended Use Cases

- Local development
- Learning / academic projects
- Proof-of-concept deployments
- Small private demos

### ❌ Not Recommended (Yet)

- Public production environments
- High-traffic deployments
- Secure competitive programming contests
- Multi-tenant untrusted code execution

---

## 🔧 Dockerfile Limitations

The current Docker setup has several **known limitations**:

- ❌ No resource isolation (CPU / memory limits not enforced)
- ❌ No secure sandboxing for untrusted code (e.g. seccomp, gVisor, Firecracker)
- ❌ No proper job queue or worker scaling
- ❌ No persistent volume strategy defined
- ❌ No health checks or monitoring
- ❌ No CI/CD pipeline integration

Because of this, **executing untrusted user code inside the current Docker setup is unsafe**.

---

## 🚀 Deployment Options (Current & Future)

### Option 1: Local Docker Compose (Current)

```bash
docker-compose up --build
