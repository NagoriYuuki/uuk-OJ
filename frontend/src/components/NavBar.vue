<template>
  <header class="navbar">
    <div class="navbar-inner">
      <div class="navbar-left">
        <RouterLink class="brand" to="/">uuk-OJ</RouterLink>
        <RouterLink class="navlink" to="/problems">题目</RouterLink>
        <RouterLink class="navlink" to="/submissions">提交</RouterLink>
        <RouterLink v-if="isLoggedIn" class="navlink" to="/me">个人主页</RouterLink>
        <template v-if="isAdmin">
          <RouterLink class="navlink" to="/problem-ma">题目管理</RouterLink>
          <RouterLink class="navlink" to="/user-ma">用户管理</RouterLink>
        </template>
      </div>

      <div class="navbar-right">
        <template v-if="isLoggedIn">
          <span class="userinfo">
            <span class="username">{{ authState.username || '已登录' }}</span>
            <span v-if="isAdmin" class="badge">Admin</span>
          </span>
          <button class="btn-ghost" @click="doLogout">退出</button>
        </template>
        <template v-else>
          <RouterLink class="navlink" to="/login">登录</RouterLink>
          <RouterLink class="navlink" to="/register">注册</RouterLink>
        </template>
      </div>
    </div>
  </header>
</template>

<script setup lang="ts">
import { computed, onMounted } from 'vue'
import { RouterLink, useRouter } from 'vue-router'
import { authState, clearSession, refreshUserInfo } from '../utils/auth'

const router = useRouter()

const isLoggedIn = computed(() => !!authState.token)
const isAdmin = computed(() => authState.role === 1)

async function doLogout() {
  clearSession()
  await router.push('/')
}

onMounted(() => {
  refreshUserInfo(false)
})
</script>

<style scoped>
.navbar {
  position: sticky;
  top: 0;
  z-index: 10;
  background: rgba(255, 255, 255, 0.75);
  backdrop-filter: blur(10px);
  border-bottom: 1px solid rgba(15, 23, 42, 0.10);
}
.navbar-inner {
  max-width: 1200px;
  margin: 0 auto;
  padding: 12px 16px;
  display: flex;
  align-items: center;
  justify-content: space-between;
  gap: 16px;
}
.navbar-left,
.navbar-right {
  display: flex;
  align-items: center;
  gap: 12px;
}
.brand {
  color: rgba(15, 23, 42, 0.92);
  text-decoration: none;
  font-weight: 700;
  margin-right: 8px;
  letter-spacing: -0.02em;
}
.navlink {
  color: rgba(15, 23, 42, 0.72);
  text-decoration: none;
  font-weight: 500;
  padding: 6px 10px;
  border-radius: 12px;
}
.navlink.router-link-active {
  color: rgba(15, 23, 42, 0.92);
  background: rgba(79, 70, 229, 0.10);
  border: 1px solid rgba(79, 70, 229, 0.18);
}
.userinfo {
  display: inline-flex;
  align-items: center;
  gap: 8px;
  color: rgba(15, 23, 42, 0.86);
}
.badge {
  font-size: 12px;
  padding: 2px 8px;
  border-radius: 999px;
  background: rgba(79, 70, 229, 0.10);
  border: 1px solid rgba(79, 70, 229, 0.22);
  color: rgba(79, 70, 229, 0.95);
}
</style>
