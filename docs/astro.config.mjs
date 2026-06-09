import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

export default defineConfig({
  site: 'https://suleymanlaarabi.github.io',
  base: '/sijson',
  integrations: [
    starlight({
      title: 'sijson',
      description: 'Documentation for the sijson C23 JSON serialization library.',
      sidebar: [
        { label: 'Overview', link: '/' },
        {
          label: 'Guide',
          items: [
            'getting-started',
            'reflected-structs',
            'dynamic-values',
            'memory-model',
            'errors-and-limits',
          ],
        },
        {
          label: 'Reference',
          items: ['reference/api'],
        },
      ],
    }),
  ],
});
