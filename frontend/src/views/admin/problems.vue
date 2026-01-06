<template>
    <div style="padding: 16px">
        <h1>题目管理</h1>
        <p>
            <RouterLink to="/">返回首页</RouterLink>
        </p>

        <button @click="$router.push({ name: 'problem-create' })">新建题目</button>

        <table style="border: 1px solid black; width: 100%; margin-top: 10px; border-collapse: collapse;">
            <thead>
                <tr>
                    <th>ID</th>
                    <th>标题</th>
                    <th>通过/提交</th>
                    <th>操作</th>
                </tr>
            </thead>
            <tbody>
                <tr v-for="p in problems" :key="p.id">
                    <td>{{ p.id }}</td>
                    <td>{{ p.title }}</td>
                    <td>{{ p.ac_count }} / {{ p.sub_count }}</td>
                    <td>
                        <button @click="$router.push({ name: 'problem-edit', params: { id: p.id } })">编辑</button>
                        <button @click="deleteProblem(p.id)">删除</button>
                    </td>
                </tr>
            </tbody>
        </table>
    </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'

interface Problem {
    id: number; title: string; ac_count: number; sub_count: number;
}

const problems = ref<Problem[]>([])

async function getProblems() {
    try {
        const res = await fetch('/api/problems')
        if (res.ok) problems.value = await res.json()
    } catch (e) {
        console.error(e)
    }
}

async function deleteProblem(id: number) {
    if (!confirm('确定删除该题目?'))
        return
    const token = localStorage.getItem('token')

    const res = await fetch(`/api/admin/problems/${id}`, {
        method: 'DELETE',
        headers: {
            'Authorization': `Bearer ${token}`
        }
    })

    if (res.ok) {
        alert('删除成功')
        getProblems()
    }
    else
        alert('删除失败')

}

onMounted(getProblems)
</script>