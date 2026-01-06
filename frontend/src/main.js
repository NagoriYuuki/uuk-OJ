import { createApp } from 'vue'
import './style.css'
import 'katex/dist/katex.min.css' // 引入 KaTeX 样式
import App from './App.vue'
import router from './router'

createApp(App).use(router).mount('#app')
