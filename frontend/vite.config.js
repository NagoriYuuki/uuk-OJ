import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'

export default defineConfig({
  plugins: [vue()],
  server: {
    host: '0.0.0.0', 
    port: 5173,
    // 【修改点】设置为 true，允许任何 host 访问
    // 这对于内网穿透开发环境非常方便
    allowedHosts: true, 
    proxy: {
      '/api': {
        target: 'http://localhost:18080', 
        changeOrigin: true,
      }
    }
  }
})