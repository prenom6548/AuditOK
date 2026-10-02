import { useState, useRef, useEffect } from 'react';
import { Puzzle, Check, Zap, Loader2 } from 'lucide-react';
import { t } from '@/lib/i18n';
import { useSkills } from '@/hooks/useApi';
import { toggleSkill as toggleSkillApi } from '@/lib/api';
import type { Skill } from '@/types/api';

function TruncatedText({ text, maxLines }: { text: string; maxLines: number }) {
  const textRef = useRef<HTMLParagraphElement>(null);
  const wrapperRef = useRef<HTMLDivElement>(null);
  const [isTruncated, setIsTruncated] = useState(false);
  const [showTooltip, setShowTooltip] = useState(false);

  useEffect(() => {
    requestAnimationFrame(() => {
      if (textRef.current) {
        setIsTruncated(textRef.current.scrollHeight > textRef.current.clientHeight);
      }
    });
  }, [text, maxLines]);

  return (
    <div
      ref={wrapperRef}
      className="relative"
      onMouseEnter={() => isTruncated && setShowTooltip(true)}
      onMouseLeave={() => setShowTooltip(false)}
    >
      <p
        ref={textRef}
        className={`text-sm mt-1 line-clamp-${maxLines}`}
        style={{ color: 'var(--pc-text-muted)' }}
      >
        {text}
      </p>
      {isTruncated && showTooltip && (
        <div
          className="absolute z-50 left-0 top-full mt-1 p-2 rounded-md text-xs whitespace-pre-wrap max-w-[300px] shadow-lg border"
          style={{
            background: '#ffffff',
            color: 'var(--pc-text-primary)',
            borderColor: 'var(--pc-border)',
          }}
        >
          {text}
        </div>
      )}
    </div>
  );
}

interface SkillWithCategory extends Skill {
  category: string;
}

function SkillSwitch({
  enabled,
  loading,
  onToggle,
}: {
  enabled: boolean;
  loading: boolean;
  onToggle: () => void;
}) {
  return (
    <button
      onClick={onToggle}
      disabled={loading}
      className="relative w-10 h-6 rounded-full transition-colors disabled:opacity-50"
      style={{
        background: enabled ? 'var(--pc-accent)' : 'var(--pc-border)',
      }}
      aria-pressed={enabled}
      aria-label={enabled ? t('skills.enabled') : t('skills.disabled')}
    >
      {loading ? (
        <Loader2 className="absolute top-1 left-1 w-4 h-4 animate-spin text-white" />
      ) : (
        <span
          className="absolute top-1 left-1 w-4 h-4 rounded-full bg-white transition-transform"
          style={{
            transform: enabled ? 'translateX(16px)' : 'translateX(0)',
          }}
        />
      )}
    </button>
  );
}

function getStatus(enabled: boolean): 'Active' | 'Available' {
  return enabled ? 'Active' : 'Available';
}

function statusBadge(status: 'Active' | 'Available') {
  switch (status) {
    case 'Active':
      return {
        icon: Check,
        label: t('skills.status_active'),
        color: 'var(--color-status-success)',
        border: 'rgba(0, 230, 138, 0.2)',
        bg: 'rgba(0, 230, 138, 0.06)',
      };
    case 'Available':
      return {
        icon: Zap,
        label: t('skills.status_available'),
        color: 'var(--pc-accent)',
        border: 'var(--pc-accent-dim)',
        bg: 'var(--pc-accent-glow)',
      };
  }
}

function getCategoryFromTags(tags: string[]): string {
  if (tags.includes('open-skills')) {
    return 'Open Skills';
  }
  return 'My Skills';
}

export default function Skills() {
  const { data: skills, loading, error, refetch } = useSkills();
  const [activeCategory, setActiveCategory] = useState<string>('all');
  const [toggling, setToggling] = useState<string | null>(null);

  const handleToggle = async (skillName: string, currentEnabled: boolean) => {
    setToggling(skillName);
    try {
      await toggleSkillApi(skillName, !currentEnabled);
      refetch();
    } catch (err) {
      console.error('Failed to toggle skill:', err);
    } finally {
      setToggling(null);
    }
  };

  const skillsWithCategory: SkillWithCategory[] = (skills || []).map((skill) => ({
    ...skill,
    category: getCategoryFromTags(skill.tags),
  }));

  const categories = ['all', ...Array.from(new Set(skillsWithCategory.map((s) => s.category))).sort()];
  const filtered = activeCategory === 'all'
    ? skillsWithCategory
    : skillsWithCategory.filter((s) => s.category === activeCategory);

  const grouped = filtered.reduce<Record<string, SkillWithCategory[]>>((acc, item) => {
    const key = item.category;
    if (!acc[key]) acc[key] = [];
    acc[key].push(item);
    return acc;
  }, {});

  if (loading) {
    return (
      <div className="p-6 flex items-center justify-center min-h-[200px]">
        <Loader2 className="h-8 w-8 animate-spin" style={{ color: 'var(--pc-accent)' }} />
      </div>
    );
  }

  if (error) {
    return (
      <div className="p-6 text-center">
        <p style={{ color: 'var(--pc-text-muted)' }}>{t('skills.empty')}</p>
      </div>
    );
  }

  return (
    <div className="p-6 space-y-6 animate-fade-in">
      <div className="flex items-center gap-2">
        <Puzzle className="h-5 w-5" style={{ color: 'var(--pc-accent)' }} />
        <h2 className="text-sm font-semibold uppercase tracking-wider" style={{ color: 'var(--pc-text-primary)' }}>
          {t('skills.title')} ({skills?.length || 0})
        </h2>
      </div>

      {/* Category Filter Tabs */}
      <div className="flex flex-wrap gap-2">
        {categories.map((cat) => (
          <button
            key={cat}
            onClick={() => setActiveCategory(cat)}
            className="px-3.5 py-1.5 rounded-[6px] text-xs font-semibold transition-all capitalize"
            style={activeCategory === cat
              ? { background: 'var(--pc-accent)', color: 'white' }
              : { color: 'var(--pc-text-muted)', border: '1px solid var(--pc-border)', background: 'transparent' }
            }
          >
            {cat === 'all' ? t('skills.all') : cat}
          </button>
        ))}
      </div>

      {/* Grouped Skill Cards */}
      {Object.keys(grouped).length === 0 ? (
        <div className="card card-radius-6 p-8 text-center">
          <Puzzle className="h-10 w-10 mx-auto mb-3" style={{ color: 'var(--pc-text-faint)' }} />
          <p style={{ color: 'var(--pc-text-muted)' }}>{t('skills.empty')}</p>
        </div>
      ) : (
        Object.entries(grouped).sort(([a], [b]) => a.localeCompare(b)).map(([category, items]) => (
          <div key={category}>
            <h3 className="text-[10px] font-semibold uppercase tracking-wider mb-3 capitalize" style={{ color: 'var(--pc-text-faint)' }}>
              {category}
            </h3>
            <div className="grid grid-cols-1 md:grid-cols-2 xl:grid-cols-3 gap-4 stagger-children">
              {items.map((skill) => {
                const status = getStatus(skill.enabled);
                const badge = statusBadge(status);
                const BadgeIcon = badge.icon;
                const isToggling = toggling === skill.name;
                return (
                  <div
                    key={skill.name}
                    className="card card-radius-6 p-5 animate-slide-in-up"
                  >
                    <div className="flex items-start justify-between gap-3">
                      <div className="min-w-0 flex-1">
                        <h4 className="text-sm font-semibold truncate" style={{ color: 'var(--pc-text-primary)' }}>
                          {skill.name}
                        </h4>
                        <TruncatedText text={skill.description} maxLines={2} />
                        {skill.version && (
                          <p className="text-xs mt-1" style={{ color: 'var(--pc-text-faint)' }}>
                            v{skill.version}
                          </p>
                        )}
                      </div>
                      <div className="flex flex-col items-end gap-2">
                        <SkillSwitch
                          enabled={skill.enabled}
                          loading={isToggling}
                          onToggle={() => handleToggle(skill.name, skill.enabled)}
                        />
                        <span
                          className="flex-shrink-0 inline-flex items-center gap-1 px-2.5 py-1 rounded-full text-[10px] font-semibold border"
                          style={badge}
                        >
                          <BadgeIcon className="h-3 w-3" />
                          {badge.label}
                        </span>
                      </div>
                    </div>
                  </div>
                );
              })}
            </div>
          </div>
        ))
      )}
    </div>
  );
}