# MATLAB Simulink 仿真模型

此目录存放 MATLAB/Simulink 仿真模型文件（`.slx` / `.mdl`），用于：

- 系统级行为仿真（闭环控制验证）
- 拓扑与调制策略评估（VSR + BUCK / SVPWM / DPWM-A）
- 算法原型开发与参数整定

---

## 推荐的 Git 配置

### `.gitattributes`

Simulink 模型文件是二进制格式，需注册为二进制以防止 diff/merge 损坏：

```
*.slx    binary
*.slxp   binary
*.mdl    binary
*.mat    binary
*.fig    binary
*.mex*   binary
```

### `.gitignore`

不要提交 Simulink 缓存和生成文件：

```gitignore
# Simulink cache
slprj/
*.slxc

# MATLAB autosave / temporary
*.asv
*.m~
*.autosave

# Code generation output
codegen/
*_ert_rtw/
*_grt_rtw/
*_slprj/

# Simulation output
*.slxc
*.slprj
*.mex*
```
