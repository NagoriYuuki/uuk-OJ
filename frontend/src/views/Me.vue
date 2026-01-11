<template>
  <div class="card">
    <h2 style="margin: 6px 0 8px;">跳转中…</h2>
    <p class="muted" style="margin: 0;">正在进入个人主页</p>
  </div>
</template>

<script setup lang="ts">
import { onMounted } from 'vue'
import { useRouter } from 'vue-router'
import { authState, refreshUserInfo } from '../utils/auth'

const router = useRouter()

onMounted(async () => {
  await refreshUserInfo(false)
  if (!authState.userId) {
    await router.replace({ name: 'login', query: { redirect: '/me' } })
    return
  }
  await router.replace({ name: 'user-profile', params: { id: authState.userId } })
})
</script>
