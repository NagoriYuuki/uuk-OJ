import { createRouter, createWebHistory } from 'vue-router'

import Index from '../views/Index.vue'
import ProblemList from '../views/ProblemList.vue'
import ProblemDetail from '../views/ProblemDetail.vue'

const router = createRouter({
    history: createWebHistory(),
    routes: [
        { path: '/', name: 'index', component: Index },
        { path: '/index', redirect: '/' },
        { path: '/problems', name: 'problem-list', component: ProblemList },
        {
            path: '/problems/:id(\\d+)',
            name: 'problem-detail',
            component: ProblemDetail,
            props: (route) => ({ id: Number(route.params.id) }),
        },
    ],
})

export default router
