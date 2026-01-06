<template>
    <div style="padding: 16px; max-width: 800px;">
        <h1>{{ isEdit ? '编辑题目' : '新建题目' }}</h1>

        <form @submit.prevent="submitForm">
            <div>
                <label>标题:</label>
                <input v-model="form.title" required style="width: 100%" />
            </div>

            <div style="display: flex; gap: 20px; margin: 10px 0;">
                <div>
                    <label>时间限制 (ms):</label>
                    <input type="number" v-model.number="form.time_limit" required />
                </div>
                <div>
                    <label>内存限制 (KB):</label>
                    <input type="number" v-model.number="form.mem_limit" required />
                </div>
            </div>

            <div style="margin-top: 10px;">
                <label>题目描述 (支持 Markdown & LaTeX):</label>

                <!-- 切换标签 -->
                <div style="margin-bottom: 5px;">
                    <button type="button" @click="previewMode = false"
                        :style="{ fontWeight: !previewMode ? 'bold' : 'normal' }">
                        编辑
                    </button>
                    <button type="button" @click="previewMode = true"
                        :style="{ fontWeight: previewMode ? 'bold' : 'normal' }">
                        预览
                    </button>
                </div>

                <!-- 编辑模式 -->
                <div v-show="!previewMode">
                    <textarea v-model="form.description" rows="15" style="width: 100%; font-family: monospace;"
                        placeholder="输入题目描述... 支持 $E=mc^2$ 公式"></textarea>
                </div>

                <!-- 预览模式 -->
                <div v-show="previewMode" class="markdown-body"
                    style="border: 1px solid #ccc; padding: 10px; min-height: 200px;" v-html="renderedDescription">
                </div>
            </div>

            <div style="margin-top: 10px;">
                <label>样例输入:</label>
                <textarea v-model="form.sample_input" rows="3" style="width: 100%"></textarea>
            </div>

            <div style="margin-top: 10px;">
                <label>样例输出:</label>
                <textarea v-model="form.sample_output" rows="3" style="width: 100%"></textarea>
            </div>

            <div style="margin-top: 10px; border: 1px dashed #ccc; padding: 10px; border-radius: 4px;">
                <label style="font-weight: bold;">上传测试数据 (ZIP):</label>
                <div style="display: flex; align-items: center; gap: 10px; margin-top: 5px;">
                    <input type="file" ref="fileInput" accept=".zip" />
                    <button type="button" @click="uploadTestcases" :disabled="uploading" v-if="isEdit">
                        {{ uploading ? '上传中...' : '上传并解压' }}
                    </button>
                    <span v-else style="color: #666; font-size: 0.9em;">
                        (请先保存题目，生成ID后再上传数据)
                    </span>
                </div>
                <div v-if="uploadMsg" :style="{ color: uploadSuccess ? 'green' : 'red', marginTop: '5px' }">
                    {{ uploadMsg }}
                </div>
            </div>

            <div style="margin-top: 10px;">
                <label>测试用例路径 (服务端路径):</label>
                <!-- 现在这个可以变成只读，或者允许手动修改 -->
                <input v-model="form.tc_path" style="width: 100%" />
            </div>

            <div style="margin-top: 20px;">
                <button type="submit" style="padding: 8px 16px;">保存提交</button>
                <button type="button" @click="$router.back()" style="margin-left: 10px;">取消</button>
            </div>
        </form>
    </div>
</template>

<script setup lang="ts">

import { ref, computed, reactive, onMounted } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { renderMarkdown } from '../../utils/markdown'

const route = useRoute()
const router = useRouter()
const previewMode = ref(false)


const problemId = route.params.id
const isEdit = computed(() => !!problemId)

const form = reactive({
    title: '',
    time_limit: 1000,
    mem_limit: 65536,
    description: '',
    sample_input: '',
    sample_output: '',
    tc_path: ''
})

const renderedDescription = computed(() => {
    return renderMarkdown(form.description)
})

async function fetchDetails() {
    if (!isEdit.value) return

    try {
        const res = await fetch(`/api/problems/${problemId}`)
        if (res.ok) {
            const data = await res.json()
            Object.assign(form, data)
        } else {
            alert("题目不存在或加载失败")
        }
    } catch (e) {
        alert("网络错误，加载题目详情失败")
    }
}

async function submitForm() {
    const token = localStorage.getItem('token')
    if (!token) return alert('请先登录')

    const url = isEdit.value
        ? `/api/admin/problems/${problemId}`
        : `/api/admin/problems`
    console.debug(url)
    const method = isEdit.value ? 'PUT' : 'POST'

    try {
        const res = await fetch(url, {
            method,
            headers: {
                'Content-Type': 'application/json',
                'Authorization': `Bearer ${token}`
            },
            body: JSON.stringify(form)
        })

        if (!res.ok) {
            const errData = await res.json().catch(() => ({}))
            const msg = errData.message || await res.text()
            alert('保存失败: ' + msg)
            return
        }

        const data = await res.json()
        const id = isEdit.value ? problemId : data.new_id

        alert('保存成功')
        router.push({
            name: 'problem-detail',
            params: { id }
        })

    } catch (e) {
        console.error(e)
        alert('请求出错')
    }

}

const fileInput = ref<HTMLInputElement | null>(null)
const uploading = ref(false)
const uploadMsg = ref('')
const uploadSuccess = ref(false)


async function uploadTestcases() {
    if (!fileInput.value?.files?.length) {
        alert('请选择一个ZIP文件')
        return
    }

    const file = fileInput.value.files[0]
    const formData = new FormData()
    formData.append('file', file)

    uploading.value = true
    uploadMsg.value = ''

    try {
        const token = localStorage.getItem('token')
        const response = await fetch(`/api/admin/problems/${problemId}/testcases`, {
            method: 'POST',
            headers: {
                'Authorization': `Bearer ${token}`
            },
            body: formData
        })

        const data = await response.json()

        if (response.ok && data.code === 200) {
            uploadSuccess.value = true
            uploadMsg.value = '上传成功！路径已更新 (请点击保存提交以持久化路径)'
            if (data.tc_path) {
                form.tc_path = data.tc_path
            }
        } else {
            throw new Error(data.message || 'Upload failed')
        }
    } catch (e: any) {
        uploadSuccess.value = false
        uploadMsg.value = '上传失败: ' + e.message
    } finally {
        uploading.value = false
    }
}

onMounted(() => {
    fetchDetails()
})
</script>

<style>
.markdown-body h1,
.markdown-body h2 {
    border-bottom: 1px solid #eee;
    margin-bottom: 10px;
}

.markdown-body p {
    margin-bottom: 10px;
    line-height: 1.6;
}

.markdown-body pre {
    background: #f6f8fa;
    padding: 16px;
    overflow: auto;
}

.markdown-body code {
    background: #f6f8fa;
    padding: 2px 4px;
}
</style>